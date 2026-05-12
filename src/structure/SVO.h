#ifndef SVO_H_INCLUDED
#define SVO_H_INCLUDED

#pragma once

#include <cstdint>

#include <structure/DynamicArray.h>

struct Node {
    uint32_t data[3];

    Node() : data{0u, 0u, 0u} {
    }

    inline void isLeaf(bool isLeaf) {
        data[0] = (data[0] & (~1u)) | isLeaf;
    }

    inline void ptr(uint32_t ptr) {
        data[0] = (data[0] & 1u) | (ptr << 1u);
    }

    inline void mask(uint64_t mask) {
        data[1] = mask;
        data[2] = mask >> 32ull;
    }

    inline bool isLeaf() {
        return data[0] & 1u;
    }

    inline uint32_t ptr() {
        return data[0] >> 1u;
    }

    inline uint64_t mask() {
        return (static_cast<uint64_t>(data[2]) << 32ull) | data[1];
    }
};

struct SVO {
    uint32_t size;
    uint32_t dim;
    DynamicArray<Node> nodes;
    DynamicArray<uint8_t> leafData;

    SVO(uint32_t size) : size(size << 1u), dim(4u << (size << 1u)) {
        nodes.emplace();
    }
};

struct IRNode {
    union {
        IRNode* children[64u];
        uint8_t leafData[64u];
    };
    uint64_t mask;
    bool isLeaf;

    IRNode(bool isLeaf) : mask(0ull), isLeaf(isLeaf) {
        if(!isLeaf) {
            std::memset(children, 0u, sizeof(children));
        }
    }

    ~IRNode() {
        if(!isLeaf) {
            for (uint32_t i = 0u; i < 64u; i++) {
                if(children[i]) {
                    delete children[i];
                }
            }
        }
    }
};

struct SVOBuilder {
    IRNode* cache[16u];
    uint32_t cacheX, cacheY, cacheZ;
    bool coldCache;

    uint32_t size;
    uint32_t dim;
    uint32_t nodeCount;

    SVOBuilder(uint32_t size) : coldCache(true), size(size << 1u), dim(4u << (size << 1u)), nodeCount(1u) {
        cache[0u] = new IRNode(size == 2u);
    }

    ~SVOBuilder() {
        if(cache[0u]) {
            delete cache[0u];
        }
    }

    void setVoxel(uint32_t x, uint32_t y, uint32_t z, uint8_t matIdx) {
        uint32_t leafX = x >> 2u;
        uint32_t leafY = y >> 2u;
        uint32_t leafZ = z >> 2u;

        uint32_t s;

        if (coldCache) {
            s = size;
            coldCache = false;
        } else {
            uint32_t diff = (cacheX ^ leafX) | (cacheY ^ leafY) | (cacheZ ^ leafZ);
            s = diff == 0u ? 0u : (((31u - __builtin_clz(diff)) & (~1u)) + 2u);
        }

        uint32_t level = (size - s) >> 1u;
        IRNode* node = cache[level];

        while (s > 0u) {
            uint32_t cx = (x >> s) & 3u;
            uint32_t cy = (y >> s) & 3u;
            uint32_t cz = (z >> s) & 3u;
            uint32_t bit = cx | (cy << 2u) | (cz << 4u);

            IRNode*& child = node->children[bit];
            if (child == nullptr) {
                child = new IRNode(s == 2u);
                nodeCount++;
            }
            node->mask |= 1ull << bit;
            node = child;

            cache[++level] = node;
            s -= 2u;
        }
        int off = (x & 3u) | ((y & 3u) << 2u) | ((z & 3u) << 4u);
        node->mask |= 1ull << off;
        node->isLeaf = true;
        node->leafData[off] = matIdx;
        cacheX = leafX;
        cacheY = leafY;
        cacheZ = leafZ;
    }

    SVO build() {
        SVO svo(size >> 1u);
        if (cache[0u] != nullptr) {
            svo.nodes.ensureCapacity(nodeCount);
            svo.nodes[0u].mask(cache[0u]->mask);
            uint32_t half = size - 2u;
            Node children[64u]{};
            uint32_t nodeIdx = 0u;
            for (uint32_t i = 0u; i < 64u; i++) {
                buildNode(half, cache[0u]->children[i], children[nodeIdx], svo);
                if (children[nodeIdx].mask() != 0ull) {
                    nodeIdx++;
                }
            }
            svo.nodes[0u].ptr(svo.nodes.size());
            svo.nodes.addAll(children, nodeIdx);
        }
        return svo;
    }

    void buildNode(uint32_t size, IRNode* ir, Node& node, SVO& svo) {
        if (size == 0u) {
            node.mask(ir == nullptr ? 0ull : ir->mask);
            node.isLeaf(true);
            node.ptr(svo.leafData.size());
            uint64_t mask = node.mask();
            if (ir != nullptr) {
                for (uint32_t i = 0u; i < 64u; i++) {
                    if ((mask & (1ull << i)) != 0ull) {
                        svo.leafData.add(ir->leafData[i]);
                    }
                }
            }
            return;
        }
        if (ir == nullptr) {
            return;
        }
        node.mask(ir->mask);
        uint32_t half = size - 2u;
        Node children[64u]{};
        uint32_t nodeIdx = 0u;
        for (uint32_t i = 0u; i < 64u; i++) {
            buildNode(half, ir->children[i], children[nodeIdx], svo);
            if (children[nodeIdx].mask() != 0ull) {
                nodeIdx++;
            }
        }
        node.ptr(svo.nodes.size());
        svo.nodes.addAll(children, nodeIdx);
    }
};

#endif