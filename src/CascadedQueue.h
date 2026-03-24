#ifndef CASCADEDQUEUE_H_INCLUDED
#define CASCADEDQUEUE_H_INCLUDED

#pragma once

#include <type_traits>
#include <cstdint>
#include "Allocator.h"
#include "DynamicArray.h"

template<typename T>
struct CascadedQueue {
private:
    DynamicArray<T*> buffers;
    DynamicArray<uint32_t> counts;
    uint32_t stages;
    uint32_t size;

    inline uint32_t getSize(uint32_t stage) {
        return 4u << stage;
    }

    void grow() {
        uint32_t nextStage = stages;
        buffers.add(alloc<T>(getSize(nextStage)));
        counts.add(0u);
        stages++;
    }

    void cascade() {
        for (int32_t stage = stages - 2; stage >= 0; stage--) {
            if (counts[stage] == 0u && counts[stage + 1] != 0u) {
                uint32_t s = getSize(stage);
                T*& a = buffers[stage];
                T*& b = buffers[stage + 1];
                uint32_t nextCount = counts[stage + 1];
                uint32_t amt = nextCount > s ? s : nextCount;
                std::memcpy(a, b, amt * sizeof(T));
                if (nextCount <= s) {
                } else {
                    uint32_t remaining = nextCount - s;
                    std::memmove(b, b + s, remaining * sizeof(T));
                }
                counts[stage] += amt;
                counts[stage + 1] -= amt;
            }
        }
    }
public:
    CascadedQueue() : buffers(), counts(), stages(1u), size(0u) {
        this->buffers.add(alloc<T>(getSize(0u)));
        this->counts.add(0u);
    }

    void add(T& e) {
        if (counts[stages - 1u] == getSize(stages - 1u)) {
            grow();
        }
        for (int32_t i = stages - 1; i >= 0; i--) {
            if (counts[i] == 0u) {
                if (i == 0) {
                    buffers[i][counts[i]++] = e;
                    size++;
                    return;
                }
                continue;
            }
            if (counts[i] == getSize(i)) {
                i++;
            }
            buffers[i][counts[i]++] = e;
            size++;
            return;
        }
    }

    void add(T&& e) {
        if (counts[stages - 1u] == getSize(stages - 1u)) {
            grow();
        }
        for (int32_t i = stages - 1; i >= 0; i--) {
            if (counts[i] == 0u) {
                if (i == 0) {
                    buffers[i][counts[i]++] = std::move(e);
                    size++;
                    return;
                }
                continue;
            }
            if (counts[i] == getSize(i)) {
                i++;
            }
            buffers[i][counts[i]++] = std::move(e);
            size++;
            return;
        }
    }

    template<typename U = T, typename... Args>
    std::enable_if_t<std::is_constructible_v<U, Args...>>
    add(Args&&... args) {
        add(U(std::forward<Args>(args)...));
    }

    template<typename U = T, typename... Args>
    std::enable_if_t<!std::is_constructible_v<U, Args...>>
    add(Args&&... args) {
        add(U{std::forward<Args>(args)...});
    }

    bool poll(T& out) {
        if (size == 0u) {
            return false;
        }
        T*& first = buffers[0u];

        //std::memcpy(&out, &first[0u], sizeof(T));
        out = first[0u];

        if (getSize(0u) > 1u) {
            std::memmove(first, first + 1u, (getSize(0u) - 1u) * sizeof(T));
        }
        counts[0u]--;
        cascade();
        size--;
        return true;
    }
};

#endif
