#ifndef STORAGE_H_INCLUDED
#define STORAGE_H_INCLUDED

#pragma once

#include <cstdint>

#include <structure/Registry.h>

template <typename T>
struct Storage {
    constexpr static bool __DEFAULT__ = true;

    DynamicArray<T> data;
    Registry reg;

    inline uint32_t add(const T& component) noexcept {
        data.add(component);
        return reg.create();
    }

    inline void set(uint32_t componentID, const T& component) noexcept {
        data[reg[componentID]] = component;
    }

    inline void remove(uint32_t componentID) noexcept {
        uint32_t loc;
        if (reg.remove(componentID, loc)) {
            data[loc] = data[data.size() - 1u];
        }
        data.removeEnd(1u);
    }

    inline bool valid(uint32_t componentID) {
        return reg.valid(componentID);
    }

    inline T& get(uint32_t id) const noexcept {
        return data[reg[id]];
    }
};

#endif
