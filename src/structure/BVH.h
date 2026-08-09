#ifndef BVH_H_INCLUDED
#define BVH_H_INCLUDED

#pragma once

#include <Allocator.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <immintrin.h>
#include <utils/perf.h>
#include <vector>

#define POSITIVE_INFINITY 100000000000.0f
#define NEGATIVE_INFINITY -100000000000.0f

#ifndef RAYFUNC
#define RAYFUNC

using RayIntersectionFunction = float (*)(uint32_t, const float*);

#endif

template <uint32_t Dim>
struct BVH {
public:
    BVH(float* bounds, uint32_t count) : bounds(bounds), nodeBounds(alloc<float>(count * 2u * Dim2)), nodeLeft(alloc<uint32_t>(count * 2u)), nodeRight(alloc<uint32_t>(count * 2u)), nodeBoxIndex(alloc<uint32_t>(count * 2u)), indices(alloc<uint32_t>(count)) {
        for (uint32_t i = 0u; i < count; i++) {
            this->indices[i] = i;
        }
        buildNode(0u, count);
        this->stack = alloc<uint32_t>(count * 2u);
    }

    ~BVH() {
        free(nodeBounds);

        free(nodeLeft);
        free(nodeRight);
        free(nodeBoxIndex);

        free(indices);
        free(stack);
    }

    uint32_t query(const float* __restrict__ query, uint32_t* __restrict__ hits, uint32_t length, uint32_t* __restrict__ stack) {
        uint32_t hitCount = 0u;
        uint32_t sp = 0u;
        stack[sp++] = 0u;

        while (sp > 0u) {
            uint32_t nodeIdx = stack[--sp];
            const float* __restrict__ nodeBound = nodeBounds + nodeIdx * Dim2;
            bool overlap = true;
            for (uint32_t i = 0; i < Dim; i++) {
                overlap &= query[i + Dim] >= nodeBound[i] && query[i] <= nodeBound[i + Dim];
            }
            if (!overlap) {
                continue;
            }
            uint32_t boxIdx = nodeBoxIndex[nodeIdx];
            if (boxIdx != UINT32_MAX) {
                hits[hitCount++] = boxIdx;
                if (hitCount >= length) {
                    return hitCount;
                }
            } else {
                uint32_t left = nodeLeft[nodeIdx];
                if (left != UINT32_MAX) {
                    __builtin_prefetch(nodeBounds + left * Dim2);
                    stack[sp++] = left;
                }
                uint32_t right = nodeRight[nodeIdx];
                if (right != UINT32_MAX) {
                    __builtin_prefetch(nodeBounds + right * Dim2);
                    stack[sp++] = right;
                }
            }
        }
        return hitCount;
    }

    inline uint32_t query(const float* __restrict__ q, uint32_t* __restrict__ hits, uint32_t length) {
        return query(q, hits, length, stack);
    }

    uint32_t queryIntersection(const float* __restrict__ pos, const float* __restrict__ dir, float& dist, const RayIntersectionFunction intersectionFunc) {
        uint32_t sp = 0u;
        stack[sp++] = 0u;
        uint32_t hitIndex = UINT32_MAX;
        float closest = POSITIVE_INFINITY;

        while (sp > 0u) {
            uint32_t nodeIdx = stack[--sp];
            const float* __restrict__ nodeBound = nodeBounds + nodeIdx * Dim2;
            if (!rayIntersectsAABB(pos, dir, nodeBound, nodeBound + Dim)) {
                continue;
            }
            uint32_t boxIdx = nodeBoxIndex[nodeIdx];
            if (boxIdx != UINT32_MAX) {
                float d = intersectionFunc(boxIdx, nodeBound);
                if (d >= 0.0f && d < closest) {
                    closest = d;
                    hitIndex = boxIdx;
                }
            } else {
                uint32_t left = nodeLeft[nodeIdx];
                if (left != UINT32_MAX) {
                    __builtin_prefetch(nodeBounds + left * Dim2);
                    stack[sp++] = left;
                }
                uint32_t right = nodeRight[nodeIdx];
                if (right != UINT32_MAX) {
                    __builtin_prefetch(nodeBounds + right * Dim2);
                    stack[sp++] = right;
                }
            }
        }
        if (hitIndex != UINT32_MAX) {
            dist = closest;
        }
        return hitIndex;
    }

private:
    static constexpr uint32_t Dim2 = Dim << 1u;
    static constexpr uint32_t BIN_COUNT = 8u;
    static constexpr float BIN_COUNT_F = static_cast<float>(BIN_COUNT);
    static constexpr uint32_t LEAF_SIZE = 8u;
    static constexpr uint32_t PARALLEL_THRESHOLD = 8u;

    inline float surfaceArea(const float* __restrict__ b) const {
        if constexpr (Dim == 2u) {
            float wx = b[2u] - b[0u];
            float wy = b[3u] - b[1u];
            if (wx < 0)
                wx = 0;
            if (wy < 0)
                wy = 0;
            return wx * wy;
        } else if constexpr (Dim == 3u) {
            float wx = b[3u] - b[0u];
            float wy = b[4u] - b[1u];
            float wz = b[5u] - b[2u];
            if (wx < 0)
                wx = 0;
            if (wy < 0)
                wy = 0;
            if (wz < 0)
                wz = 0;
            return 2.0f * (wx * wy + wx * wz + wy * wz);
        } else {
            float sides[Dim];
            for (uint32_t i = 0u; i < Dim; i++) {
                sides[i] = b[i + Dim] - b[i];
            }
            float sum = 0.0f;
            for (uint32_t i = 0u; i < Dim - 1u; i++) {
                float s = 0.0f;
                for (uint32_t j = i + 1u; j < Dim; j++) {
                    s += sides[j];
                }
                sum += sides[i] * s;
            }
            return sum * (Dim - 1.0f);
        }
    }

    const float* const bounds;

    float* const nodeBounds;
    uint32_t* const nodeLeft;
    uint32_t* const nodeRight;
    uint32_t* const nodeBoxIndex;

    uint32_t* const indices;
    uint32_t nodeCount = 0u;
    uint32_t* stack;

    uint32_t buildNode(uint32_t start, uint32_t end) {
        uint32_t nodeIdx = nodeCount++;
        float* __restrict__ nodeBound = nodeBounds + nodeIdx * Dim2;
        if (end - start == 1u) {
            uint32_t box = indices[start];
            std::memcpy(nodeBound, bounds + box * Dim2, Dim2 * sizeof(float));
            nodeLeft[nodeIdx] = UINT32_MAX;
            nodeRight[nodeIdx] = UINT32_MAX;
            nodeBoxIndex[nodeIdx] = box;
            return nodeIdx;
        }
        for (uint32_t i = 0u; i < Dim; i++) {
            nodeBound[i] = POSITIVE_INFINITY;
            nodeBound[i + Dim] = NEGATIVE_INFINITY;
        }
        if (nodeIdx == 0u) {
            alignas(32u) float buffer[Dim2 * 8u];
            alignas(32u) float b[8u * Dim2];
            __m256 bv[Dim2];
            for (uint32_t i = 0u; i < Dim2; i++) {
                bv[i] = _mm256_set1_ps(nodeBound[i]);
            }
            uint32_t it = start;
            for (; it + 7u < end; it += 8u) {
                std::memcpy(buffer, bounds + it * Dim2, 8u * Dim2 * sizeof(float));
                for (uint32_t j = 0u; j < Dim2; j++) {
                    const uint32_t k = j * 8u;
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
                    const uint32_t idx = j + Dim;
                    bv[idx] = _mm256_max_ps(bv[idx], _mm256_load_ps(b + idx * 8u));
                }
            }
            for (uint32_t i = 0u; i < Dim; i++) {
                __m256 v = bv[i];
                __m128 m = _mm_min_ps(_mm256_castps256_ps128(v), _mm256_extractf128_ps(v, 1u));
                m = _mm_min_ps(m, _mm_movehl_ps(m, m));
                m = _mm_min_ps(m, _mm_shuffle_ps(m, m, 1u));
                nodeBound[i] = _mm_cvtss_f32(m);
                v = bv[i + Dim];
                m = _mm_max_ps(_mm256_castps256_ps128(v), _mm256_extractf128_ps(v, 1u));
                m = _mm_max_ps(m, _mm_movehl_ps(m, m));
                m = _mm_max_ps(m, _mm_shuffle_ps(m, m, 1u));
                nodeBound[i + Dim] = _mm_cvtss_f32(m);
            }
            for (; it < end; it++) {
                const float* __restrict__ b = bounds + it * Dim2;
                for (uint32_t j = 0u; j < Dim; j++) {
                    nodeBound[j] = std::min(nodeBound[j], b[j]);
                    uint32_t idx = j + Dim;
                    nodeBound[idx] = std::max(nodeBound[idx], b[idx]);
                }
            }
        } else {
            for (uint32_t i = start; i < end; i++) {
                const float* __restrict__ b = bounds + indices[i] * Dim2;
                for (uint32_t j = 0; j < Dim; j++) {
                    nodeBound[j] = std::min(nodeBound[j], b[j]);
                    uint32_t idx = j + Dim;
                    nodeBound[idx] = std::max(nodeBound[idx], b[idx]);
                }
            }
        }
        nodeBoxIndex[nodeIdx] = UINT32_MAX;

        uint32_t axis = 0u;
        float maxSize = 0.0f;
        for (uint32_t i = 0u; i < Dim; i++) {
            float size = nodeBound[i + Dim] - nodeBound[i];
            if (size > maxSize) {
                maxSize = size;
                axis = i;
            }
        }
        uint32_t mid = (start + end) >> 1u;
        quickSelect(start, end - 1u, mid, axis);
        if (mid == start) {
            mid++;
        }
        if (mid == end) {
            mid--;
        }
        uint32_t left = buildNode(start, mid);
        uint32_t right = buildNode(mid, end);
        nodeLeft[nodeIdx] = left;
        nodeRight[nodeIdx] = right;
        return nodeIdx;
    }

    /*uint32_t buildNode(const uint32_t start, const uint32_t end) {
        //uint64_t time[13];
        //time[0] = rdtsc();
        const uint32_t n = end - start;
        const uint32_t center = (start + end) >> 1;
        const uint32_t nodeIdx = nodeCount++;
        float* const nodeBound = nodeBounds + nodeIdx * Dim2;

        if (n == 1) {
            const uint32_t box = indices[start];
            std::memcpy(nodeBound, bounds + box * Dim2, sizeof(float) * Dim2);
            nodeLeft[nodeIdx] = -1;
            nodeRight[nodeIdx] = -1;
            nodeBoxIndex[nodeIdx] = box;
            return nodeIdx;
        }
        //time[1] = rdtsc();
        for (uint32_t i = 0; i < Dim; i++) {
            nodeBound[i] = POSITIVE_INFINITY;
            nodeBound[i + Dim] = NEGATIVE_INFINITY;
        }
        //time[2] = rdtsc();
        for (uint32_t ii = start; ii < end; ii++) {
            const float* b = bounds + indices[ii] * Dim2;
            for (uint32_t i = 0; i < Dim; i++) {
                float mn = b[i];
                float mx = b[i + Dim];
                if (mn < nodeBound[i]) {
                    nodeBound[i] = mn;
                }
                if (mx > nodeBound[i + Dim]) {
                    nodeBound[i + Dim] = mx;
                }
            }
        }
        nodeBoxIndex[nodeIdx] = -1;
        //time[3] = rdtsc();
        uint32_t splitAxis = 0;
        float maxExtent = nodeBound[Dim] - nodeBound[0];
        for (uint32_t i = 1; i < Dim; i++) {
            float ext = nodeBound[i + Dim] - nodeBound[i];
            if (ext > maxExtent) {
                maxExtent = ext;
                splitAxis = i;
            }
        }
        const float* boundsC = bounds + splitAxis;
        //time[4] = rdtsc();

        float parentArea = surfaceArea(nodeBound);
        //time[5] = rdtsc();
        if (parentArea <= 0.0f || maxExtent <= 1e-9f) {
            std::nth_element(indices + start, indices + center, indices + end, [&](uint32_t a, uint32_t b) {
                const float* A = boundsC + a * Dim2;
                const float* B = boundsC + b * Dim2;
                return A[0] + A[Dim] < B[0] + B[Dim];
            });
            uint32_t left = buildNode(start, center);
            uint32_t right = buildNode(center, end);
            nodeLeft[nodeIdx] = left;
            nodeRight[nodeIdx] = right;
            return nodeIdx;
        }

        uint32_t binCount[BIN_COUNT]{};
        alignas(16) float binBounds[BIN_COUNT * Dim2];

        for (uint32_t i = 0; i < BIN_COUNT * Dim2; i += Dim2) {
            for(uint32_t j = 0; j < Dim; j++) {
                binBounds[i + j] = POSITIVE_INFINITY;
                binBounds[i + Dim + j] = NEGATIVE_INFINITY;
            }
        }

        const float axisMin = nodeBound[splitAxis];
        const float axisExtent = nodeBound[splitAxis + Dim] - nodeBound[splitAxis];
        const float invExtent = 1.0f / axisExtent;

        //time[6] = rdtsc();

        for (uint32_t ii = start; ii < end; ii++) {
            const float* b = bounds + indices[ii] * Dim2;
            float mid = 0.5f * (b[splitAxis] + b[splitAxis + Dim]);
            const uint32_t binIdx = std::clamp(static_cast<uint32_t>((mid - axisMin) * invExtent * BIN_COUNT), 0, BIN_COUNT - 1);
            const uint32_t bi = binIdx * Dim2;
            //if constexpr (Dim == 2) {
            //    __m128 V0 = _mm_load_ps(b);
            //    __m128 V1 = _mm_load_ps(binBounds + bi);
            //    _mm_blend_ps(_mm_min_ps(V0, V1), _mm_max_ps(V0, V1), 0b1100);
            //} else {
                for (uint32_t i = 0u; i < Dim; i++) {
                    binBounds[bi + i] = std::min(binBounds[bi + i], b[i]);
                    binBounds[bi + Dim + i] = std::max(binBounds[bi + Dim + i], b[i + Dim]);
                }
            //}
            binCount[binIdx]++;
        }

        //time[7] = rdtsc();

        uint32_t leftCount[BIN_COUNT];
        alignas(16) float leftBounds[BIN_COUNT][Dim2];
        uint32_t rightCount[BIN_COUNT];
        alignas(16) float rightBounds[BIN_COUNT][Dim2];

        //time[8] = rdtsc();

        uint32_t acc = 0;
        bool leftInit = false;
        for (uint32_t i = 0u; i < BIN_COUNT; i++) {
            acc += binCount[i];
            leftCount[i] = acc;
            if (binCount[i] == 0u) {
                if(leftInit) {
                    std::memcpy(leftBounds[i], leftBounds[i - 1u], Dim2 * sizeof(float));
                }
            } else {
                if(leftInit) {
                    for (uint32_t j = 0u; j < Dim; j++) {
                        leftBounds[i][j] = std::min(leftBounds[i - 1u][j], binBounds[i * Dim2 + j]);
                        leftBounds[i][j + Dim] = std::max(leftBounds[i - 1u][j + Dim], binBounds[i * Dim2 + Dim + j]);
                    }
                } else {
                    std::memcpy(leftBounds[i], binBounds + i * Dim2, Dim2 * sizeof(float));
                    for (uint32_t k = 0u; k < i; k++) {
                        std::memcpy(leftBounds[k], leftBounds[i], Dim2 * sizeof(float));
                    }
                    leftInit = true;
                }
            }
        }

        //time[9] = rdtsc();

        acc = 0;
        bool rightInit = false;
        for (uint32_t i = BIN_COUNT - 1; i >= 0; i--) {
            acc += binCount[i];
            rightCount[i] = acc;
            if (binCount[i] == 0u) {
                if (rightInit) {
                    std::memcpy(rightBounds[i], rightBounds[i + 1], Dim2 * sizeof(float));
                }
            } else {
                if (rightInit) {
                    for (uint32_t j = 0; j < Dim; j++) {
                        rightBounds[i][j] = std::min(rightBounds[i + 1][j], binBounds[i * Dim2 + j]);
                        rightBounds[i][j + Dim] = std::max(rightBounds[i + 1][j + Dim], binBounds[i * Dim2 + Dim + j]);
                    }
                } else {
                    std::memcpy(rightBounds[i], binBounds + i * Dim2, Dim2 * sizeof(float));
                    for (uint32_t k = BIN_COUNT - 1; k > i; k--) {
                        std::memcpy(rightBounds[k], rightBounds[i], Dim2 * sizeof(float));
                    }
                    rightInit = true;
                }
            }
        }

        //time[10] = rdtsc();

        float bestCost = POSITIVE_INFINITY;
        uint32_t bestSplit = -1;
        for (uint32_t i = 0; i < BIN_COUNT - 1; ++i) {
            uint32_t nL = leftCount[i];
            uint32_t nR = rightCount[i + 1];
            if (nL == 0 || nR == 0) {
                continue;
            }
            float leftSA = surfaceArea(leftBounds[i]);
            float rightSA = surfaceArea(rightBounds[i + 1]);
            float cost = (leftSA / parentArea) * float(nL) + (rightSA / parentArea) * float(nR);
            if (cost < bestCost) {
                bestCost = cost;
                bestSplit = i;
            }
        }

        //time[11] = rdtsc();

        uint32_t mid;
        if (bestSplit == -1) {
            mid = center;
            std::nth_element(indices + start, indices + center, indices + end, [&](uint32_t a, uint32_t b) {
                const float* A = boundsC + a * Dim2;
                const float* B = boundsC + b * Dim2;
                return A[0] + A[Dim] < B[0] + B[Dim];
            });
        } else {
            float splitPos = axisMin + (axisExtent * static_cast<float>(bestSplit + 1) / BIN_COUNT_F);
            uint32_t i = start;
            uint32_t j = end - 1;
            while (i <= j) {
                const float* b = bounds + indices[i] * Dim2;
                float center = 0.5f * (b[splitAxis] + b[splitAxis + Dim]);
                if (center < splitPos) {
                    i++;
                } else {
                    swapIdx(i, j);
                    j--;
                }
            }
            mid = i;
            if (mid == start || mid == end) {
                mid = center;
                std::nth_element(indices + start, indices + center, indices + end, [&](uint32_t a, uint32_t b) {
                    const float* A = boundsC + a * Dim2;
                    const float* B = boundsC + b * Dim2;
                    return A[0] + A[Dim] < B[0] + B[Dim];
                });
            }
        }

        //time[12] = rdtsc();
        uint32_t left = buildNode(start, mid);
        uint32_t right = buildNode(mid, end);
        nodeLeft[nodeIdx] = left;
        nodeRight[nodeIdx] = right;

        //uint32_t maxIdx = 0;
        //uint64_t maxT = 0u;
        //for(uint32_t i = 0u; i < 12u; i++) {
        //    uint64_t t = time[i + 1u] - time[i];
        //    if(t > maxT) {
        //        maxT = t;
        //        maxIdx = i;
        //    }
        //}
        //std::cout << maxIdx << " " << maxT << std::endl;
        return nodeIdx;
    }*/

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

    void quickSelect(uint32_t left, uint32_t right, uint32_t k, uint32_t axis) {
        while (left < right) {
            uint32_t pivot = partition(left, right, axis);
            if (k < pivot) {
                right = pivot - 1;
            } else if (k > pivot) {
                left = pivot + 1;
            } else {
                return;
            }
        }
    }

    uint32_t partition(uint32_t low, uint32_t high, uint32_t axis) {
        const float* pivot = bounds + indices[high] * Dim2;
        float pivotVal = (pivot[axis] + pivot[axis + Dim]) * 0.5f;
        uint32_t i = low - 1;
        for (uint32_t j = low; j < high; j++) {
            const float* b = bounds + indices[j] * Dim2;
            float center = (b[axis] + b[axis + Dim]) * 0.5f;
            if (center <= pivotVal) {
                i++;
                swapIdx(i, j);
            }
        }
        swapIdx(i + 1, high);
        return i + 1;
    }

    inline void swapIdx(uint32_t a, uint32_t b) {
        uint32_t tmp = indices[a];
        indices[a] = indices[b];
        indices[b] = tmp;
    }
};

#endif
