#ifndef HEAPBUFFER_H_INCLUDED
#define HEAPBUFFER_H_INCLUDED

#pragma once

#include <cstdint>
#include <cstring>
#include <structure/BitSet.h>

template<typename T>
struct HeapBuffer {
    T* const data;
    uint32_t* const blockSizes;
    const uint32_t length;
    const uint32_t unitSize;
    BitSet memoryMask;

    HeapBuffer(const uint32_t length, const uint32_t unitSize = 1u, const uint32_t alignment = 32u) : data(reinterpret_cast<T*>(_mm_malloc(length * unitSize * sizeof(T), alignment))), blockSizes(reinterpret_cast<uint32_t*>(_mm_malloc(length * sizeof(uint32_t), 32u))), length(length), unitSize(unitSize) {
    }

    ~HeapBuffer() {
        _mm_free(data);
        _mm_free(blockSizes);
    }

    inline void mov(const uint32_t ptr, const T* const v, uint32_t units) {
        std::memcpy(data + ptr * unitSize, v, units * unitSize * sizeof(T));
    }

    uint32_t locate(const uint32_t size, const uint32_t fixedIncrement) {
        uint32_t currentIdx = 0u;
        while (currentIdx < length) {
            uint32_t startIdx = memoryMask.nextClearBit(currentIdx);
            if (startIdx >= length) {
                break;
            }
            startIdx = ((startIdx + fixedIncrement - 1u) / fixedIncrement) * fixedIncrement;
            uint32_t endIdx = memoryMask.nextSetBit(startIdx);
            if (endIdx == 0xFFFFFFFFu || endIdx > length) {
                endIdx = length;
            }
            if ((endIdx - startIdx) >= size) {
                memoryMask.set(startIdx, startIdx + size);
                return startIdx;
            }
            currentIdx = endIdx + 1u;
        }
        return 0xFFFFFFFFu;
    }

    uint32_t reserve(const uint32_t units, const uint32_t fixedIncrement = 1u) {
        uint32_t ptr = locate(units, fixedIncrement);
        blockSizes[ptr] = units;
        return ptr;
    }

    uint32_t allocate(const T* const v, const uint32_t units, const uint32_t fixedIncrement = 1u) {
        uint32_t ptr = reserve(units, fixedIncrement);
        mov(ptr, v, units);
        return ptr;
    }

    uint32_t relocate(const uint32_t ptr, const uint32_t units, const uint32_t fixedIncrement = 1u) {
        const uint32_t currentSize = blockSizes[ptr];
        if(units == currentSize) {
            return ptr;
        }
        blockSizes[ptr] = units;
        if(units < currentSize) {
            memoryMask.clear(ptr + units, ptr + currentSize);
            return ptr;
        }
        uint32_t endIdx = memoryMask.nextSetBit(ptr + currentSize);
        if (endIdx == 0xFFFFFFFFu || endIdx > length) {
            endIdx = length;
        }
        if (endIdx - ptr >= units) {
            memoryMask.set(ptr, ptr + units);
            return ptr;
        }

        uint32_t newPtr = locate(units, fixedIncrement);
        blockSizes[newPtr] = units;
        memoryMask.clear(ptr, ptr + currentSize);
        return newPtr;
    }

    uint32_t reallocate(const uint32_t ptr, const T* const v, const uint32_t units, const uint32_t fixedIncrement = 1u) {
        uint32_t newPtr = relocate(ptr, units, fixedIncrement);
        mov(newPtr, v, units);
        return newPtr;
    }

    void free(const uint32_t ptr) {
        memoryMask.clear(ptr, ptr + blockSizes[ptr]);
    }

    inline T* pointer(uint32_t ptr) const {
        return data + ptr * unitSize;
    }
};

#endif
