#ifndef STORAGE_H_INCLUDED
#define STORAGE_H_INCLUDED

#pragma once

#include <cstdint>

#include <structure/Registry.h>
#include <structure/VirtualArray.h>

template <typename T>
struct Storage {
    Registry reg;
    DynamicArray<uint32_t> entitySet;

    VirtualArray<T> data;

    Storage() : reg(), entitySet(), data() {
    }

    inline uint32_t add(T&& component, uint32_t entityID) noexcept {
        data.emplace(std::move(component));
        entitySet.add(entityID);
        return reg.create();
    }

    inline void set(uint32_t componentID, T&& component) noexcept {
        data[reg[componentID]] = std::move(component);
    }

    inline void remove(uint32_t componentID) noexcept {
        uint32_t loc;
        if (reg.remove(componentID, loc)) {
            uint32_t end = data.size() - 1u;
            data[loc] = std::move(data[end]);
            entitySet[loc] = entitySet[end];
        }
        data.removeEnd(1u);
        entitySet.removeEnd(1u);
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
