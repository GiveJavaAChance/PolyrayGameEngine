#ifndef DYNAMICBVH_H_INCLUDED
#define DYNAMICBVH_H_INCLUDED

#pragma once

#include <algorithm>
#include <cstdint>
#include <immintrin.h>

#include <structure/DynamicArray.h>

#define POSITIVE_INFINITY 100000000000.0f
#define NEGATIVE_INFINITY -100000000000.0f

using RayIntersectionFunction = float (*)(uint32_t, const float*);

template <uint32_t Dim>
struct DynamicBVH {
private:
    constexpr static uint32_t Dim2 = Dim * 2u;

    struct Node {
        float bounds[Dim2];
        uint32_t left;
        uint32_t right;

        uint32_t boxIndex;
    };

    uint32_t nodeCount;
    DynamicArray<Node> nodes;

    DynamicArray<uint32_t> indices;

    DynamicArray<uint32_t> stack;

    uint32_t buildNode(const float* __restrict__ bounds, uint32_t start, uint32_t end) {
        uint32_t nodeIdx = nodeCount++;
        Node* node = nodes + nodeIdx;
        if (end - start == 1u) {
            uint32_t box = indices[start];
            std::memcpy(node->bounds, bounds + box * Dim2, Dim2 * sizeof(float));
            node->left = UINT32_MAX;
            node->right = UINT32_MAX;
            node->boxIndex = box;
            return nodeIdx;
        }
        for (uint32_t i = 0u; i < Dim; i++) {
            node->bounds[i] = POSITIVE_INFINITY;
            node->bounds[i + Dim] = NEGATIVE_INFINITY;
        }
        if (nodeIdx == 0u) {
            alignas(32u) float buffer[Dim2 * 8u];
            alignas(32u) float b[8u * Dim2];
            __m256 bv[Dim2];
            for (uint32_t i = 0u; i < Dim2; i++) {
                bv[i] = _mm256_set1_ps(node->bounds[i]);
            }
            uint32_t it = start;
            for (; it + 7u < end; it += 8u) {
                std::memcpy(buffer, bounds + it * Dim2, 8u * Dim2 * sizeof(float));
                for (uint32_t j = 0; j < Dim2; j++) {
                    uint32_t k = j * 8u;
                    b[0u + k] = buffer[0u * Dim2 + j];
                    b[1u + k] = buffer[1u * Dim2 + j];
                    b[2u + k] = buffer[2u * Dim2 + j];
                    b[3u + k] = buffer[3u * Dim2 + j];
                    b[4u + k] = buffer[4u * Dim2 + j];
                    b[5u + k] = buffer[5u * Dim2 + j];
                    b[6u + k] = buffer[6u * Dim2 + j];
                    b[7u + k] = buffer[7u * Dim2 + j];
                }
                for (uint32_t j = 0u; j < Dim; j++) {
                    bv[j] = _mm256_min_ps(bv[j], _mm256_load_ps(b + j * 8u));
                    uint32_t idx = j + Dim;
                    bv[idx] = _mm256_max_ps(bv[idx], _mm256_load_ps(b + idx * 8u));
                }
            }
            for (uint32_t i = 0u; i < Dim; i++) {
                __m256 v = bv[i];
                __m128 m = _mm_min_ps(_mm256_castps256_ps128(v), _mm256_extractf128_ps(v, 1u));
                m = _mm_min_ps(m, _mm_movehl_ps(m, m));
                m = _mm_min_ps(m, _mm_shuffle_ps(m, m, 1u));
                node->bounds[i] = _mm_cvtss_f32(m);
                v = bv[i + Dim];
                m = _mm_max_ps(_mm256_castps256_ps128(v), _mm256_extractf128_ps(v, 1u));
                m = _mm_max_ps(m, _mm_movehl_ps(m, m));
                m = _mm_max_ps(m, _mm_shuffle_ps(m, m, 1u));
                node->bounds[i + Dim] = _mm_cvtss_f32(m);
            }
            for (; it < end; it++) {
                const float* __restrict__ b = bounds + it * Dim2;
                for (uint32_t j = 0u; j < Dim; j++) {
                    node->bounds[j] = std::min(node->bounds[j], b[j]);
                    uint32_t idx = j + Dim;
                    node->bounds[idx] = std::max(node->bounds[idx], b[idx]);
                }
            }
        } else {
            for (uint32_t i = start; i < end; i++) {
                const float* __restrict__ b = bounds + indices[i] * Dim2;
                for (uint32_t j = 0u; j < Dim; j++) {
                    node->bounds[j] = std::min(node->bounds[j], b[j]);
                    uint32_t idx = j + Dim;
                    node->bounds[idx] = std::max(node->bounds[idx], b[idx]);
                }
            }
        }
        node->boxIndex = UINT32_MAX;

        uint32_t axis = 0u;
        float maxSize = 0.0f;
        for (uint32_t i = 0u; i < Dim; i++) {
            float size = node->bounds[i + Dim] - node->bounds[i];
            if (size > maxSize) {
                maxSize = size;
                axis = i;
            }
        }
        uint32_t mid = (start + end) >> 1u;
        quickSelect(bounds, start, end - 1u, mid, axis);
        if (mid == start) {
            mid++;
        }
        if (mid == end) {
            mid--;
        }
        uint32_t left = buildNode(bounds, start, mid);
        uint32_t right = buildNode(bounds, mid, end);
        node->left = left;
        node->right = right;
        return nodeIdx;
    }

    bool rayIntersectsAABB(const float* __restrict__ pos, const float* __restrict__ invDir, const float* __restrict__ min, const float* __restrict__ max) {
        float tMin = NEGATIVE_INFINITY;
        float tMax = POSITIVE_INFINITY;
        for (uint32_t i = 0u; i < Dim; i++) {
            float t0 = (min[i] - pos[i]) * invDir[i];
            float t1 = (max[i] - pos[i]) * invDir[i];
            if (invDir[i] < 0.0f) {
                float tmp = t0;
                t0 = t1;
                t1 = tmp;
            }
            tMin = t0 > tMin ? t0 : tMin;
            tMax = t1 < tMax ? t1 : tMax;
            if (tMax < tMin) {
                return false;
            }
        }
        return tMax >= 0.0f;
    }

    void quickSelect(const float* __restrict__ bounds, uint32_t left, uint32_t right, uint32_t k, uint32_t axis) {
        while (left < right) {
            uint32_t pivot = partition(bounds, left, right, axis);
            if (k < pivot) {
                right = pivot - 1u;
            } else if (k > pivot) {
                left = pivot + 1u;
            } else {
                return;
            }
        }
    }

    uint32_t partition(const float* __restrict__ bounds, uint32_t low, uint32_t high, uint32_t axis) {
        const float* __restrict__ pivot = bounds + indices[high] * Dim2;
        float pivotVal = (pivot[axis] + pivot[axis + Dim]) * 0.5f;
        uint32_t i = low - 1u;
        for (uint32_t j = low; j < high; j++) {
            const float* __restrict__ b = bounds + indices[j] * Dim2;
            float center = (b[axis] + b[axis + Dim]) * 0.5f;
            if (center <= pivotVal) {
                i++;
                swapIdx(i, j);
            }
        }
        swapIdx(i + 1u, high);
        return i + 1u;
    }

    inline void swapIdx(uint32_t a, uint32_t b) {
        uint32_t tmp = indices[a];
        indices[a] = indices[b];
        indices[b] = tmp;
    }

public:
    void build(const float* __restrict__ bounds, uint32_t count) {
        indices.ensureCapacity(count);
        for (uint32_t i = 0u; i < count; i++) {
            indices[i] = i;
        }
        nodeCount = 0u;
        nodes.ensureCapacity(count * 2u);
        buildNode(bounds, 0u, count);
    }

    uint32_t query(const float* __restrict__ query, uint32_t* __restrict__ hits, uint32_t length, uint32_t* __restrict__ stack) {
        uint32_t hitCount = 0u;
        uint32_t sp = 0u;
        stack[sp++] = 0u;

        while (sp > 0u) {
            uint32_t nodeIdx = stack[--sp];
            Node node = nodes[nodeIdx];
            bool overlap = true;
            for (uint32_t i = 0u; i < Dim; i++) {
                overlap &= query[i + Dim] >= node.bounds[i] && query[i] <= node.bounds[i + Dim];
            }
            if (!overlap) {
                continue;
            }
            if (node.boxIndex != UINT32_MAX) {
                hits[hitCount++] = node.boxIndex;
                if (hitCount >= length) {
                    return hitCount;
                }
            } else {
                if (node.left != UINT32_MAX) {
                    stack[sp++] = node.left;
                }
                if (node.right != UINT32_MAX) {
                    stack[sp++] = node.right;
                }
            }
        }
        return hitCount;
    }

    inline uint32_t query(const float* __restrict__ q, uint32_t* __restrict__ hits, uint32_t length) {
        return query(q, hits, length, stack.data());
    }

    uint32_t queryIntersection(const float* __restrict__ pos, const float* __restrict__ dir, float& dist, const RayIntersectionFunction intersectionFunc) {
        uint32_t sp = 0u;
        stack[sp++] = 0u;
        uint32_t hitIndex = UINT32_MAX;
        float closest = POSITIVE_INFINITY;

        float invDir[Dim];
        for(uint32_t i = 0u; i < Dim; i++) {
            invDir[i] = 1.0f / dir[i];
        }
        while (sp > 0u) {
            uint32_t nodeIdx = stack[--sp];
            Node node = nodes[nodeIdx];
            if (!rayIntersectsAABB(pos, invDir, node.bounds, node.bounds + Dim)) {
                continue;
            }
            if (node.boxIndex != UINT32_MAX) {
                float d = intersectionFunc(node.boxIndex, node.bounds);
                if (d >= 0.0f && d < closest) {
                    closest = d;
                    hitIndex = node.boxIndex;
                }
            } else {
                if (node.left != UINT32_MAX) {
                    stack[sp++] = node.left;
                }
                if (node.right != UINT32_MAX) {
                    stack[sp++] = node.right;
                }
            }
        }
        if (hitIndex != UINT32_MAX) {
            dist = closest;
        }
        return hitIndex;
    }
};

#endif
