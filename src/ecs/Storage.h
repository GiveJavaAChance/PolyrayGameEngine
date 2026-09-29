#ifndef STORAGE_H_INCLUDED
#define STORAGE_H_INCLUDED

#pragma once

#include <cstdint>

#include <structure/Registry.h>
#include <structure/VirtualArray.h>

template <typename T>
struct Storage {
    constexpr static bool __DEFAULT__ = true;

    VirtualArray<T> data;
    Registry reg;

    inline uint32_t add(T&& component) noexcept {
        data.emplace(std::move(component));
        return reg.create();
    }

    inline void set(uint32_t componentID, T&& component) noexcept {
        data[reg[componentID]] = std::move(component);
    }

    inline void remove(uint32_t componentID) noexcept {
        uint32_t loc;
        if (reg.remove(componentID, loc)) {
            data[loc] = std::move(data[data.size() - 1u]);
        }
        data.removeEnd(1u);
    }

    inline bool valid(uint32_t componentID) {
        return reg.valid(componentID);
    }

    inline T& get(uint32_t id) noexcept {
        return data[reg[id]];
    }

    inline const T& get(uint32_t id) const noexcept {
        return data[reg[id]];
    }
};

#endif
