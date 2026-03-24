#ifndef BITSET_H_INCLUDED
#define BITSET_H_INCLUDED

#pragma once

#include <cstdint>
#include <climits>
#include <cstring>
#include "DynamicArray.h"

struct BitSet {
private:
    DynamicArray<uint64_t> words;
    uint32_t size;

    inline uint32_t numberOfTrailingZeros64(uint64_t x) {
        return x == 0ull ? 64u : __builtin_ctzll(x);
    }

    inline void ensureCapacity(uint32_t cap) {
        if(cap > size) {
            words.ensureCapacity(cap);
            std::memset(words.data() + size, 0, (cap - size) * sizeof(uint64_t));
            size = cap;
        }
    }

public:
    struct Iterator {
        const DynamicArray<uint64_t>& words;
        uint32_t wordIndex;
        uint32_t bitIndex;
        uint64_t currentWord;
        uint32_t maxWord;

        Iterator(const DynamicArray<uint64_t>& words, uint32_t wi, uint32_t mw) : words(words), wordIndex(wi), bitIndex(0u), currentWord(0ull), maxWord(mw) {
            advanceToNextSetBit();
        }

        uint32_t operator*() const {
            return ((wordIndex - 1u) << 6u) | bitIndex;
        }

        Iterator& operator++() {
            currentWord &= currentWord - 1ull;
            advanceToNextSetBit();
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return wordIndex != other.wordIndex || currentWord != other.currentWord;
        }

    private:
        void advanceToNextSetBit() {
            while (true) {
                if (currentWord) {
                    bitIndex = __builtin_ctzll(currentWord);
                    return;
                }
                if (wordIndex >= maxWord) {
                    return;
                }
                currentWord = words[wordIndex++];
            }
        }
    };

    BitSet() : size(1u) {
        std::memset(words.data(), 0, size * sizeof(uint64_t));
    }

    inline void set(uint32_t idx) {
        uint32_t u = idx >> 6u;
        ensureCapacity(u + 1u);
        words[u] |= 1ull << idx;
    }

    void set(uint32_t start, uint32_t end) {
        uint32_t startWordIndex = start >> 6u;
        uint32_t endWordIndex = (end - 1u) >> 6u;
        ensureCapacity(endWordIndex + 1u);
        uint64_t firstWordMask = 0xFFFFFFFFFFFFFFFFull << start;
        uint64_t lastWordMask = 0xFFFFFFFFFFFFFFFFull >> (-end);
        if (startWordIndex == endWordIndex) {
            words[startWordIndex] |= (firstWordMask & lastWordMask);
        } else {
            words[startWordIndex] |= firstWordMask;
            for (uint32_t i = startWordIndex + 1u; i < endWordIndex; i++) {
                words[i] = 0xFFFFFFFFFFFFFFFFull;
            }
            words[endWordIndex] |= lastWordMask;
        }
    }

    inline void clear(uint32_t idx) {
        uint32_t u = idx >> 6u;
        ensureCapacity(u + 1u);
        words[u] &= ~(1ull << idx);
    }

    void clear(uint32_t start, uint32_t end) {
        uint32_t startWordIndex = start >> 6u;
        uint32_t endWordIndex = (end - 1u) >> 6u;
        ensureCapacity(endWordIndex + 1u);
        uint64_t firstWordMask = 0xFFFFFFFFFFFFFFFFull << start;
        uint64_t lastWordMask = 0xFFFFFFFFFFFFFFFFull >> (-end);
        if (startWordIndex == endWordIndex) {
            words[startWordIndex] &= ~(firstWordMask & lastWordMask);
        } else {
            words[startWordIndex] &= ~firstWordMask;
            for (uint32_t i = startWordIndex + 1u; i < endWordIndex; i++) {
                words[i] = 0ull;
            }
            words[endWordIndex] &= ~lastWordMask;
        }
    }

    inline void flip(uint32_t idx) {
        uint32_t u = idx >> 6u;
        ensureCapacity(u + 1u);
        words[u] ^= 1ull << idx;
    }

    void flip(uint32_t start, uint32_t end) {
        uint32_t startWordIndex = start >> 6u;
        uint32_t endWordIndex = (end - 1u) >> 6u;
        ensureCapacity(endWordIndex + 1u);
        uint64_t firstWordMask = 0xFFFFFFFFFFFFFFFFull << start;
        uint64_t lastWordMask = 0xFFFFFFFFFFFFFFFFull >> (-end);
        if (startWordIndex == endWordIndex) {
            words[startWordIndex] ^= (firstWordMask & lastWordMask);
        } else {
            words[startWordIndex] ^= firstWordMask;
            for (uint32_t i = startWordIndex + 1u; i < endWordIndex; i++) {
                words[i] ^= 0xFFFFFFFFFFFFFFFFull;
            }
            words[endWordIndex] ^= lastWordMask;
        }
    }

    uint32_t nextSetBit(uint32_t fromIndex) {
        uint32_t u = fromIndex >> 6u;
        if (u >= size) {
            return 0xFFFFFFFFu;
        }
        uint64_t word = words[u] & (0xFFFFFFFFFFFFFFFFull << fromIndex);
        while (true) {
            if (word != 0ull) {
                return (u << 6u) + numberOfTrailingZeros64(word);
            }
            if (++u == size) {
                return 0xFFFFFFFFu;
            }
            word = words[u];
        }
    }

    uint32_t nextClearBit(uint32_t fromIndex) {
        uint32_t u = fromIndex >> 6u;
        if (u >= size) {
            return fromIndex;
        }
        uint64_t word = ~words[u] & (0xFFFFFFFFFFFFFFFFull << fromIndex);
        while (true) {
            if (word != 0ull) {
                return (u << 6u) + numberOfTrailingZeros64(word);
            }
            if (++u == size) {
                return u << 6u;
            }
            word = ~words[u];
        }
    }

    inline void clear() {
        std::memset(words.data(), 0, size * sizeof(uint64_t));
        size = 0u;
    }

    uint32_t getCount() const {
        uint32_t count = 0u;
        for(uint32_t i = 0u; i < size; i++) {
            __builtin_popcountll(words[i]);
        }
        return count;
    }

    inline bool operator[](uint32_t idx) {
        return words[idx >> 6u] & (1ull << idx);
    }

    inline DynamicArray<uint64_t>& getWords() {
        return words;
    }

    Iterator begin() const {
        return Iterator(words, 0, size);
    }

    Iterator end() const {
        return Iterator(words, size, size);
    }
};

#endif
