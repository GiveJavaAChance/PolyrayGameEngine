#ifndef SHADERBUFFERHEAP_H_INCLUDED
#define SHADERBUFFERHEAP_H_INCLUDED

#pragma once

#include <cstdint>
#include <unordered_set>

#include "Allocator.h"
#include "base/glad/glad.h"
#include "ShaderBuffer.h"
#include "UnorderedRegistry.h"

struct ShaderBufferHeap {
private:
    UnorderedRegistry<uint64_t> blocks;
    std::unordered_set<uint32_t> active;
    std::unordered_set<uint32_t> updated;

    void set(uint32_t start, uint32_t end) {
        uint32_t s = start >> 6u;
        uint32_t sbit = start & 63u;
        uint32_t e = end >> 6u;
        uint32_t ebit = end & 63u;
        if (s == e) {
            uint64_t mask = ((~0ull >> (63u - (ebit + sbit))) << sbit);
            memoryMask[s] |= mask;
            return;
        }
        memoryMask[s] |= (~0ull << sbit);
        for (uint32_t i = s + 1u; i < e; i++) {
            memoryMask[i] = ~0ull;
        }
        memoryMask[e] |= ((1ull << (ebit + 1u)) - 1u);
    }

    void clear(uint32_t start, uint32_t end) {
        uint32_t s = start >> 6u;
        uint32_t sbit = start & 63u;
        uint32_t e = end >> 6u;
        uint32_t ebit = end & 63u;
        if (s == e) {
            uint64_t mask = ((~0ull >> (63u - (ebit - sbit))) << sbit);
            memoryMask[s] &= ~mask;
            return;
        }
        memoryMask[s] &= ~(~0ull << sbit);
        for (uint32_t i = s + 1u; i < e; i++) {
            memoryMask[i] = 0u;
        }
        memoryMask[e] &= ~((1ull << (ebit + 1u)) - 1u);
    }

    uint32_t nextClearBit(uint32_t from) {
        uint32_t wordIndex = from >> 6u;
        uint32_t bit = from & 63u;
        uint32_t wordCount = (memorySize + 63u) >> 6u;
        uint64_t word = ~memoryMask[wordIndex] & (~0ull << bit);
        while (true) {
            if (word != 0ull) {
                uint32_t result = (wordIndex << 6u) + __builtin_ctzll(word);
                return result < memorySize ? result : memorySize;
            }
            wordIndex++;
            if (wordIndex >= wordCount) {
                return memorySize;
            }
            word = ~memoryMask[wordIndex];
        }
    }

    uint32_t nextSetBit(uint32_t from) {
        uint32_t wordIndex = from >> 6u;
        uint32_t bit = from & 63u;
        uint32_t wordCount = (memorySize + 63u) >> 6u;
        uint64_t word = memoryMask[wordIndex] & (~0ull << bit);
        while (true) {
            if (word != 0ull) {
                uint32_t result = (wordIndex << 6u) + __builtin_ctzll(word);
                return result < memorySize ? result : memorySize;
            }
            wordIndex++;
            if (wordIndex >= wordCount) {
                return memorySize;
            }
            word = memoryMask[wordIndex];
        }
    }

    uint32_t locate(uint32_t size) {
        uint32_t currentIdx = 0u;
        while (currentIdx < memorySize) {
            uint32_t startIdx = nextClearBit(currentIdx);
            if (startIdx == memorySize) {
                break;
            }
            uint32_t endIdx = nextSetBit(startIdx);
            if ((endIdx - startIdx) >= size) {
                set(startIdx, startIdx + size);
                return startIdx;
            }
            currentIdx = endIdx + 1u;
        }
        std::cerr << "Out of memory!" << std::endl;
        return 0u;
    }

public:
    ShaderBuffer buffer;
    const uint32_t memorySize;
    const uint32_t unitSize;
    uint64_t* memoryMask;

    ShaderBufferHeap(uint32_t length, uint32_t unitSize) : buffer(GL_DYNAMIC_DRAW), memorySize(length), unitSize(unitSize), memoryMask(alloc<uint64_t>((length + 63u) >> 6u)) {
        buffer.setSize((length * unitSize) << 2u);
    }

    uint32_t reserve(uint32_t units) {
        uint32_t ptr = locate(units);
        return blocks.add(static_cast<uint64_t>(ptr) << 32ull | units);
    }

    template<typename T>
    uint32_t allocate(T* data, uint32_t length) {
        static_assert(sizeof(T) == 4u);
        uint32_t size = length / unitSize;
        uint32_t ptr = locate(size);
        uint32_t ID = blocks.add(static_cast<uint64_t>(ptr) << 32ull | size);
        buffer.uploadPartialData(data, length, ptr * unitSize);
        active.insert(ID);
        updated.insert(ID);
        return ID;
    }

    template<typename T>
    void reallocate(uint32_t ID, T* data, uint32_t length) {
        static_assert(sizeof(T) == 4u);
        uint64_t& m = blocks[ID];
        uint32_t newSize = length / unitSize;
        uint32_t ptr = static_cast<uint32_t>(m >> 32ull);
        uint32_t size = static_cast<uint32_t>(m & 0xFFFFFFFF);
        if (newSize == size) {
            buffer.uploadPartialData(data, length, ptr * unitSize);
            updated.insert(ID);
            return;
        }
        if (newSize < size) {
            clear(ptr + newSize, ptr + size);
            m = static_cast<uint64_t>(ptr) << 32ull | newSize;
            buffer.uploadPartialData(data, length, ptr * unitSize);
            updated.insert(ID);
            return;
        }
        uint32_t endIdx = nextSetBit(ptr + size);
        if (endIdx > memorySize) {
            endIdx = memorySize;
        }
        if (endIdx - ptr >= newSize) {
            set(ptr, ptr + newSize);
            m = static_cast<uint64_t>(ptr) << 32ull | newSize;
            buffer.uploadPartialData(data, length, ptr * unitSize);
            updated.insert(ID);
            return;
        }
        uint32_t newPtr = locate(newSize);
        clear(ptr, ptr + size);
        m = static_cast<uint64_t>(newPtr) << 32ull | newSize;
        buffer.uploadPartialData(data, length, newPtr * unitSize);
        updated.insert(ID);
    }

    template<typename T>
    void write(uint32_t ID, uint32_t offset, T* data, uint32_t length) {
        static_assert(sizeof(T) == 4u);
        uint64_t m = blocks[ID];
        uint32_t newSize = length / unitSize;
        uint32_t ptr = static_cast<uint32_t>(m >> 32ull);
        uint32_t size = static_cast<uint32_t>(m & 0xFFFFFFFF);
        if (offset + newSize > size) {
            std::cerr << "Writing out of bounds!" << std::endl;
            return;
        }
        buffer.uploadPartialData(data, length, (ptr + offset) * unitSize);
        updated.insert(ID);
    }

    void free(uint32_t ID) {
        uint64_t m = blocks[ID];
        blocks.remove(ID);
        uint32_t ptr = static_cast<uint32_t>(m >> 32ull);
        uint32_t size = static_cast<uint32_t>(m & 0xFFFFFFFF);
        clear(ptr, ptr + size);
        active.erase(ID);
        updated.erase(ID);
    }

    void activate(uint32_t ID) {
        if (!active.count(ID)) {
            active.insert(ID);
            updated.insert(ID);
        }
    }

    void deactivate(uint32_t ID) {
        if (active.count(ID)) {
            active.erase(ID);
            updated.insert(ID);
        }
    }

    uint32_t getTotalCount() const {
        return blocks.size();
    }

    uint32_t getActiveCount() const {
        return active.size();
    }

    uint32_t getUpdateCount() const {
        return updated.size();
    }

    void pollUpdates(uint32_t* ids = nullptr) {
        if(ids) {
            uint32_t idx = 0u;
            for (int i : updated) {
                ids[idx++] = i;
            }
        }
        updated.clear();
    }

    UnorderedRegistry<uint64_t>& getAllocations() {
        return blocks;
    }

    std::unordered_set<uint32_t>& getActive() {
        return active;
    }

    /*std::vector<uint64_t> getChunks() {

        BitSet mask = new BitSet(memorySize);
        for (int ID : active) {
            long m = blocks.get(ID);
            int ptr = (int) (m >>> 32l);
            int size = (int) (m & 0xFFFFFFFF);
            mask.set(ptr, ptr + size);
        }
        ArrayList<Long> chunks = new ArrayList<>();
        uint32_t idx = 0u;
        while (true) {
            uint32_t ptr = mask.nextSetBit(idx);
            if (ptr < 0) {
                break;
            }
            uint32_t end = mask.nextClearBit(ptr);
            if (end < 0) {
                end = memorySize;
            }
            uint32_t size = end - ptr;
            long packed = (long) ptr << 32l | (long) size;
            chunks.add(packed);
            idx = end;
        }
        return chunks;
    }*/
};

#endif
