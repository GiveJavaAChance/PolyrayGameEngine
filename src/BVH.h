#ifndef BVH_H_INCLUDED
#define BVH_H_INCLUDED

#pragma once

#include <cstdint>
#include <cstring>
#include <immintrin.h>
#include <vector>
#include <algorithm>
#include "Allocator.h"
#include "perf.h"

#define POSITIVE_INFINITY 100000000000.0f
#define NEGATIVE_INFINITY -100000000000.0f

using RayIntersectionFunction = float(*)(const int, const float*);

template<int Dim>
struct BVH {
public:
    BVH(float* bounds, int count) : numBounds(count), bounds(bounds), numNodes(count * 2), nodeBounds(alloc<float>(numNodes * Dim2)), nodeLeft(alloc<int>(numNodes)), nodeRight(alloc<int>(numNodes)), nodeBoxIndex(alloc<int>(numNodes)), indices(alloc<int>(count)) {
        for (int i = 0; i < count; i++) {
            this->indices[i] = i;
        }
        buildNode(0, count);
        this->stack = alloc<int>(numNodes);
    }

    ~BVH() {
        free(nodeBounds);

        free(nodeLeft);
        free(nodeRight);
        free(nodeBoxIndex);

        free(indices);
        free(stack);
    }

    int query(const float* const query, int* const hits, const int length, int* const stack) {
        int hitCount = 0;
        int sp = 0;
        stack[sp++] = 0;

        float qMin[Dim];
        float qMax[Dim];
        std::memcpy(qMin, query, Dim * sizeof(float));
        std::memcpy(qMax, query + Dim, Dim * sizeof(float));

        while (sp > 0) {
            int nodeIdx = stack[--sp];
            const float* nodeBound = nodeBounds + nodeIdx * Dim2;
            bool overlap = true;
            for (int i = 0; i < Dim; i++) {
                overlap &= qMax[i] >= nodeBound[i] && qMin[i] <= nodeBound[i + Dim];
            }
            if (!overlap) {
                continue;
            }
            const int boxIdx = nodeBoxIndex[nodeIdx];
            if (boxIdx != -1) {
                hits[hitCount++] = boxIdx;
                if(hitCount >= length) {
                    return hitCount;
                }
            } else {
                const int left = nodeLeft[nodeIdx];
                if (left != -1) {
                    __builtin_prefetch(nodeBounds + left * Dim2);
                    stack[sp++] = left;
                }
                const int right = nodeRight[nodeIdx];
                if (right != -1) {
                    __builtin_prefetch(nodeBounds + right * Dim2);
                    stack[sp++] = right;
                }
            }
        }
        return hitCount;
    }

    inline int query(const float* const q, int* const hits, const int length) {
        return query(q, hits, length, stack);
    }

    int queryIntersection(const float* const pos, const float* const dir, float& dist, const RayIntersectionFunction intersectionFunc) {
        int sp = 0;
        stack[sp++] = 0;
        int hitIndex = -1;
        float closest = POSITIVE_INFINITY;

        while (sp > 0) {
            int nodeIdx = stack[--sp];
            const float* nodeBound = nodeBounds + nodeIdx * Dim2;
            if (!rayIntersectsAABB(pos, dir, nodeBound)) {
                continue;
            }
            int boxIdx = nodeBoxIndex[nodeIdx];
            if (boxIdx != -1) {
                float d = intersectionFunc(boxIdx, nodeBound);
                if (d >= 0.0f && d < closest) {
                    closest = d;
                    hitIndex = boxIdx;
                }
            } else {
                const int left = nodeLeft[nodeIdx];
                if (left != -1) {
                    __builtin_prefetch(nodeBounds + left * Dim2);
                    stack[sp++] = left;
                }
                const int right = nodeRight[nodeIdx];
                if (right != -1) {
                    __builtin_prefetch(nodeBounds + right * Dim2);
                    stack[sp++] = right;
                }
            }
        }
        if (hitIndex != -1) {
            dist = closest;
        }
        return hitIndex;
    }

private:
    static constexpr int Dim2 = Dim << 1;
    static constexpr int BIN_COUNT = 8;
    static constexpr float BIN_COUNT_F = static_cast<float>(BIN_COUNT);
    static constexpr int LEAF_SIZE = 8;
    static constexpr int PARALLEL_THRESHOLD = 8;

    inline float surfaceArea(const float* b) const {
        if constexpr (Dim == 2) {
            float wx = b[2] - b[0];
            float wy = b[3] - b[1];
            if (wx < 0) wx = 0;
            if (wy < 0) wy = 0;
            return wx * wy;
        } else if constexpr (Dim == 3) {
            float wx = b[3] - b[0];
            float wy = b[4] - b[1];
            float wz = b[5] - b[2];
            if (wx < 0) wx = 0;
            if (wy < 0) wy = 0;
            if (wz < 0) wz = 0;
            return 2.0f * (wx * wy + wx * wz + wy * wz);
        } else {
            float sides[Dim];
            for(int i = 0; i < Dim; i++) {
                sides[i] = b[i + Dim] - b[i];
            }
            float sum = 0.0f;
            for(int i = 0; i < Dim - 1; i++) {
                float s = 0.0f;
                for(int j = i + 1; j < Dim; j++) {
                    s += sides[j];
                }
                sum += sides[i] * s;
            }
            return sum * (Dim - 1.0f);
        }
    }

    const int numBounds;
    const float* const bounds;

    const int numNodes;
    float* const nodeBounds;
    int* const nodeLeft;
    int* const nodeRight;
    int* const nodeBoxIndex;

    int* const indices;
    int nodeCount = 0;
    int* stack;

    int buildNode(const int start, const int end) {
        int nodeIdx = nodeCount++;
        float* nodeBound = nodeBounds + nodeIdx * Dim2;
        if (end - start == 1) {
            const int box = indices[start];
            std::memcpy(nodeBound, bounds + box * Dim2, Dim2 * sizeof(float));
            nodeLeft[nodeIdx] = -1;
            nodeRight[nodeIdx] = -1;
            nodeBoxIndex[nodeIdx] = box;
            return nodeIdx;
        }

        if(nodeIdx == 0) {
            alignas(32) float buffer[Dim2 * 8];
            alignas(32) float b[8 * Dim2];
            __m256 bv[Dim2];
            for (int i = 0; i < Dim2; i++) {
                bv[i] = _mm256_set1_ps(nodeBound[i]);
            }
            int it = start;
            for (; it + 7 < end; it += 8) {
                std::memcpy(buffer, bounds + it * Dim2, 8 * Dim2 * sizeof(float));
                for(int j = 0; j < Dim2; j++) {
                    const int k = j * 8;
                    b[0 + k] = buffer[0 * Dim2 + j];
                    b[1 + k] = buffer[1 * Dim2 + j];
                    b[2 + k] = buffer[2 * Dim2 + j];
                    b[3 + k] = buffer[3 * Dim2 + j];
                    b[4 + k] = buffer[4 * Dim2 + j];
                    b[5 + k] = buffer[5 * Dim2 + j];
                    b[6 + k] = buffer[6 * Dim2 + j];
                    b[7 + k] = buffer[7 * Dim2 + j];
                }
                for (int j = 0; j < Dim; j++) {
                    bv[j] = _mm256_min_ps(bv[j], _mm256_load_ps(b + j * 8));
                    const int idx = j + Dim;
                    bv[idx] = _mm256_max_ps(bv[idx], _mm256_load_ps(b + idx * 8));
                }
            }
            for(int i = 0; i < Dim; i++) {
                __m256 v = bv[i];
                __m128 m = _mm_min_ps(_mm256_castps256_ps128(v), _mm256_extractf128_ps(v, 1));
                m = _mm_min_ps(m, _mm_movehl_ps(m, m));
                m = _mm_min_ps(m, _mm_shuffle_ps(m, m, 1));
                nodeBound[i] = _mm_cvtss_f32(m);
                v = bv[i + Dim];
                m = _mm_max_ps(_mm256_castps256_ps128(v), _mm256_extractf128_ps(v, 1));
                m = _mm_max_ps(m, _mm_movehl_ps(m, m));
                m = _mm_max_ps(m, _mm_shuffle_ps(m, m, 1));
                nodeBound[i + Dim] = _mm_cvtss_f32(m);
            }
            for (; it < end; it++) {
                const float* b = bounds + it * Dim2;
                for (int j = 0; j < Dim; j++) {
                    nodeBound[j] = std::min(nodeBound[j], b[j]);
                    const int idx = j + Dim;
                    nodeBound[idx] = std::max(nodeBound[idx], b[idx]);
                }
            }
        } else {
            for (int i = 0; i < Dim; i++) {
                nodeBound[i] = POSITIVE_INFINITY;
                nodeBound[i + Dim] = NEGATIVE_INFINITY;
            }
            for (int i = start; i < end; i++) {
                const float* b = bounds + indices[i] * Dim2;
                for (int j = 0; j < Dim; j++) {
                    nodeBound[j] = std::min(nodeBound[j], b[j]);
                    const int idx = j + Dim;
                    nodeBound[idx] = std::max(nodeBound[idx], b[idx]);
                }
            }
        }
        nodeBoxIndex[nodeIdx] = -1;

        int axis = -1;
        float maxSize = 0.0f;
        for (int i = 0; i < Dim; i++) {
            float size = nodeBound[i + Dim] - nodeBound[i];
            if (size > maxSize) {
                maxSize = size;
                axis = i;
            }
        }
        int mid = (start + end) >> 1;
        quickSelect(start, end - 1, mid, axis);

        if (mid == start) {
            mid++;
        }
        if (mid == end) {
            mid--;
        }
        int left = buildNode(start, mid);
        int right = buildNode(mid, end);
        nodeLeft[nodeIdx] = left;
        nodeRight[nodeIdx] = right;
        return nodeIdx;
    }

    /*int buildNode(const int start, const int end) {
        //uint64_t time[13];
        //time[0] = rdtsc();
        const int n = end - start;
        const int center = (start + end) >> 1;
        const int nodeIdx = nodeCount++;
        float* const nodeBound = nodeBounds + nodeIdx * Dim2;

        if (n == 1) {
            const int box = indices[start];
            std::memcpy(nodeBound, bounds + box * Dim2, sizeof(float) * Dim2);
            nodeLeft[nodeIdx] = -1;
            nodeRight[nodeIdx] = -1;
            nodeBoxIndex[nodeIdx] = box;
            return nodeIdx;
        }
        //time[1] = rdtsc();
        for (int i = 0; i < Dim; i++) {
            nodeBound[i] = POSITIVE_INFINITY;
            nodeBound[i + Dim] = NEGATIVE_INFINITY;
        }
        //time[2] = rdtsc();
        for (int ii = start; ii < end; ii++) {
            const float* b = bounds + indices[ii] * Dim2;
            for (int i = 0; i < Dim; i++) {
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
        int splitAxis = 0;
        float maxExtent = nodeBound[Dim] - nodeBound[0];
        for (int i = 1; i < Dim; i++) {
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
            std::nth_element(indices + start, indices + center, indices + end, [&](int a, int b) {
                const float* A = boundsC + a * Dim2;
                const float* B = boundsC + b * Dim2;
                return A[0] + A[Dim] < B[0] + B[Dim];
            });
            int left = buildNode(start, center);
            int right = buildNode(center, end);
            nodeLeft[nodeIdx] = left;
            nodeRight[nodeIdx] = right;
            return nodeIdx;
        }

        uint32_t binCount[BIN_COUNT]{};
        alignas(16) float binBounds[BIN_COUNT * Dim2];

        for (int i = 0; i < BIN_COUNT * Dim2; i += Dim2) {
            for(int j = 0; j < Dim; j++) {
                binBounds[i + j] = POSITIVE_INFINITY;
                binBounds[i + Dim + j] = NEGATIVE_INFINITY;
            }
        }

        const float axisMin = nodeBound[splitAxis];
        const float axisExtent = nodeBound[splitAxis + Dim] - nodeBound[splitAxis];
        const float invExtent = 1.0f / axisExtent;

        //time[6] = rdtsc();

        for (int ii = start; ii < end; ii++) {
            const float* b = bounds + indices[ii] * Dim2;
            float mid = 0.5f * (b[splitAxis] + b[splitAxis + Dim]);
            const int binIdx = std::clamp(static_cast<int>((mid - axisMin) * invExtent * BIN_COUNT), 0, BIN_COUNT - 1);
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

        int leftCount[BIN_COUNT];
        alignas(16) float leftBounds[BIN_COUNT][Dim2];
        int rightCount[BIN_COUNT];
        alignas(16) float rightBounds[BIN_COUNT][Dim2];

        //time[8] = rdtsc();

        int acc = 0;
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
        for (int i = BIN_COUNT - 1; i >= 0; i--) {
            acc += binCount[i];
            rightCount[i] = acc;
            if (binCount[i] == 0u) {
                if (rightInit) {
                    std::memcpy(rightBounds[i], rightBounds[i + 1], Dim2 * sizeof(float));
                }
            } else {
                if (rightInit) {
                    for (int j = 0; j < Dim; j++) {
                        rightBounds[i][j] = std::min(rightBounds[i + 1][j], binBounds[i * Dim2 + j]);
                        rightBounds[i][j + Dim] = std::max(rightBounds[i + 1][j + Dim], binBounds[i * Dim2 + Dim + j]);
                    }
                } else {
                    std::memcpy(rightBounds[i], binBounds + i * Dim2, Dim2 * sizeof(float));
                    for (int k = BIN_COUNT - 1; k > i; k--) {
                        std::memcpy(rightBounds[k], rightBounds[i], Dim2 * sizeof(float));
                    }
                    rightInit = true;
                }
            }
        }

        //time[10] = rdtsc();

        float bestCost = POSITIVE_INFINITY;
        int bestSplit = -1;
        for (int i = 0; i < BIN_COUNT - 1; ++i) {
            int nL = leftCount[i];
            int nR = rightCount[i + 1];
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

        int mid;
        if (bestSplit == -1) {
            mid = center;
            std::nth_element(indices + start, indices + center, indices + end, [&](int a, int b) {
                const float* A = boundsC + a * Dim2;
                const float* B = boundsC + b * Dim2;
                return A[0] + A[Dim] < B[0] + B[Dim];
            });
        } else {
            float splitPos = axisMin + (axisExtent * static_cast<float>(bestSplit + 1) / BIN_COUNT_F);
            int i = start;
            int j = end - 1;
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
                std::nth_element(indices + start, indices + center, indices + end, [&](int a, int b) {
                    const float* A = boundsC + a * Dim2;
                    const float* B = boundsC + b * Dim2;
                    return A[0] + A[Dim] < B[0] + B[Dim];
                });
            }
        }

        //time[12] = rdtsc();
        int left = buildNode(start, mid);
        int right = buildNode(mid, end);
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


    bool rayIntersectsAABB(const float* const pos, const float* const dir, const float* const bounds) {
        float tMin = NEGATIVE_INFINITY;
        float tMax = POSITIVE_INFINITY;
        for (int i = 0; i < Dim; i++) {
            float invD = 1.0f / dir[i];
            float t0 = (bounds[i] - pos[i]) * invD;
            float t1 = (bounds[i + Dim] - pos[i]) * invD;
            if (invD < 0) {
                float tmp = t0;
                t0 = t1;
                t1 = tmp;
            }
            if (t0 > tMin) {
                tMin = t0;
            }
            if (t1 < tMax) {
                tMax = t1;
            }
            if (tMax < tMin) {
                return false;
            }
        }
        return tMax >= 0.0f;
    }

    void quickSelect(int left, int right, int k, int axis) {
        while (left < right) {
            int pivot = partition(left, right, axis);
            if (k < pivot) {
                right = pivot - 1;
            } else if (k > pivot) {
                left = pivot + 1;
            } else {
                return;
            }
        }
    }

    int partition(int low, int high, int axis) {
        const float* pivot = bounds + indices[high] * Dim2;
        float pivotVal = (pivot[axis] + pivot[axis + Dim]) * 0.5f;
        int i = low - 1;
        for (int j = low; j < high; j++) {
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

    inline void swapIdx(int a, int b) {
        int tmp = indices[a];
        indices[a] = indices[b];
        indices[b] = tmp;
    }
};

#endif
