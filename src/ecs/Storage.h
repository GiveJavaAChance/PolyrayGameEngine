#ifndef STORAGE_H_INCLUDED
#define STORAGE_H_INCLUDED

#pragma once

#include <cstdint>
#include <ecs/Component.h>
#include <structure/Registry.h>

enum StorageDataLayout : uint8_t {
    DEFAULT,
    CUSTOM
};

template<Component T>
struct StorageInfo {
    static constexpr StorageDataLayout layout = StorageDataLayout::DEFAULT;
};

template<Component T, StorageDataLayout L = StorageDataLayout::DEFAULT>
struct Storage;

template<Component T>
struct Storage<T, StorageDataLayout::DEFAULT> {
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
        reg.remove(componentID, data);
        data.removeEnd(1u);
    }

    inline T& get(uint32_t id) const noexcept {
        return data[reg[id]];
    }
};

#endif