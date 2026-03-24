#ifndef UNORDEREDREGISTRY_H_INCLUDED
#define UNORDEREDREGISTRY_H_INCLUDED

#pragma once

#include <vector>
#include "Registry.h"

template<typename T>
struct UnorderedRegistry {
    std::vector<T> arr;
    Registry reg;

    UnorderedRegistry() noexcept {
    }

    template<typename... Args>
    inline uint32_t emplace(Args&&... args) {
        arr.emplace_back(args...);
        return reg.create();
    }

    inline uint32_t add(const T& e) {
        arr.push_back(e);
        return reg.create();
    }

    inline uint32_t add(T&& e) {
        arr.push_back(e);
        return reg.create();
    }

    inline void remove(const uint32_t ID) {
        reg.remove(ID, arr);
        arr.pop_back();
    }

    inline T* data() {
        return arr.data();
    }

    inline uint32_t size() const {
        return arr.size();
    }

    inline T& operator[](uint32_t ID) {
        return arr[reg[ID]];
    }
};

#endif
