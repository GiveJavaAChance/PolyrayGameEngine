#ifndef IDGENERATOR_H_INCLUDED
#define IDGENERATOR_H_INCLUDED

#pragma once

#include <cstdint>
#include <climits>
#include <structure/DynamicArray.h>

struct IDGenerator {
private:
    DynamicArray<uint64_t> words;
    uint32_t count = 0u;

    inline uint32_t numberOfTrailingZeros64(uint64_t x) {
        return x == 0ull ? 64u : __builtin_ctzll(x);
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

    IDGenerator() {
        this->words.add(0ull);
    }

    uint32_t getNewID() {
        count++;
        uint64_t word = ~words[0u];
        uint32_t u = 0u;
        while (true) {
            if(word != 0ull) {
                uint32_t off = numberOfTrailingZeros64(word);
                words[u] |= 1ull << off;
                return (u << 6u) + off;
            }
            if (++u == words.size()) {
                words.add(0ull);
                words[u] |= 1ull;
                return u << 6u;
            }
            word = ~words[u];
        }
    }

    void free(uint32_t ID) {
        uint32_t u = ID >> 6u;
        if(u >= words.size()) {
            return;
        }
        const uint64_t mask = 1ull << (ID & 63u);
        uint64_t& word = words[u];
        if(mask & word) {
            word ^= mask;
            count--;
        }
    }

    inline void clear() {
        words.clear(true);
        words.add(0ull);
        count = 0;
    }

    inline uint32_t getCount() const {
        return count;
    }

    inline DynamicArray<uint64_t>& getWords() {
        return words;
    }

    Iterator begin() const {
        return Iterator(words, 0, words.size());
    }

    Iterator end() const {
        return Iterator(words, words.size(), words.size());
    }
};

#endif
