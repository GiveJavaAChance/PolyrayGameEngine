#ifndef VIRTUALARRAY_H_INCLUDED
#define VIRTUALARRAY_H_INCLUDED

#pragma once

#include <cstdint>
#include <type_traits>

#include <utils/VirtualMemory.h>
#include <utils/prvl_assert.h>

template <typename T>
struct VirtualArray {
private:
    VirtualMemory::MemoryInfo info;
    T* data = nullptr;
    size_t reserved = 0ull;
    size_t committed = 0ull;
    size_t pos = 0ull;

    static constexpr bool isPow2(size_t size) {
        return size && !(size & (size - 1));
    }

    static constexpr size_t roundToPow2Size(size_t minimumSize, size_t pow2Size) {
        return (minimumSize + pow2Size - 1ull) & ~(pow2Size - 1ull);
    }

    T* alloc(size_t count) {
        size_t newSize = pos + count;
        if (newSize * sizeof(T) > reserved) {
            return nullptr;
        }

        if (newSize * sizeof(T) > committed) {
            size_t newCommitted = roundToPow2Size(newSize * sizeof(T), info.pageSize);
            if (newCommitted > reserved) {
                newCommitted = reserved;
            }
            if (!VirtualMemory::commit(reinterpret_cast<uint8_t*>(data) + committed, newCommitted - committed)) {
                return nullptr;
            }
            committed = newCommitted;
        }

        T* ptr = data + pos;
        pos = newSize;
        return ptr;
    }

    void destroy() {
        if (data) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                for (size_t i = 0ull; i < pos; i++) {
                    data[i].~T();
                }
            }
            VirtualMemory::release(data, reserved);
        }
        data = nullptr;
        reserved = 0ull;
        committed = 0ull;
        pos = 0ull;
    }

public:
    explicit VirtualArray(size_t reserveSize = 1ull << 30u) : info(VirtualMemory::memoryInfo()) {
        reserved = roundToPow2Size(reserveSize, info.allocationGranularity);
        data = reinterpret_cast<T*>(VirtualMemory::reserve(reserved));
        if (!data) {
            reserved = 0ull;
        }
        committed = 0ull;
        pos = 0ull;
    }

    ~VirtualArray() {
        destroy();
    }

    VirtualArray(const VirtualArray&) = delete;
    VirtualArray& operator=(const VirtualArray&) = delete;

    VirtualArray(VirtualArray&& other)
        : info(VirtualMemory::memoryInfo()), data(other.data), reserved(other.reserved),
          committed(other.committed), pos(other.pos) {
        other.data = nullptr;
        other.reserved = 0ull;
        other.committed = 0ull;
        other.pos = 0ull;
    }

    VirtualArray& operator=(VirtualArray&& other) {
        if (this != &other) {
            destroy();
            other.data = nullptr;
            other.reserved = 0ull;
            other.committed = 0ull;
            other.pos = 0ull;
        }
        return *this;
    }

    template <typename... Args>
    inline T& emplace(Args&&... args) {
        T* ptr = alloc(1ull);
        PRVL_ASSERT(ptr, "Allocation failed")
        if constexpr (std::is_aggregate_v<T>) {
            ::new (ptr) T{std::forward<Args>(args)...};
        } else {
            ::new (ptr) T(std::forward<Args>(args)...);
        }
        return *ptr;
    }

    inline T& add(const T& e) {
        return emplace(e);
    }

    inline T& add(T&& e) {
        return emplace(std::move(e));
    }

    inline T* addAll(const T* p, size_t count) {
        T* ptr = alloc(count);
        PRVL_ASSERT(ptr, "Allocation failed")
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy(ptr, p, count * sizeof(T));
        } else {
            for (size_t i = 0ull; i < count; i++) {
                ::new (ptr + i) T(p[i]);
            }
        }
        return ptr;
    }

    inline T* reserve(size_t count) {
        T* ptr = alloc(count);
        PRVL_ASSERT(ptr, "Allocation failed")
        return ptr;
    }

    inline void removeEnd(size_t count = 1u) {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (size_t i = pos - count; i < pos; i++) {
                data[i].~T();
            }
        }
        pos -= count;
    }

    inline void clear(bool releasePages = false) {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (size_t i = 0ull; i < pos; i++) {
                data[i].~T();
            }
        }
        if (releasePages && committed) {
            VirtualMemory::decommit(data, committed);
            committed = 0ull;
        }
        pos = 0ull;
    }

    inline size_t size() const {
        return pos;
    }

    inline T* begin() {
        return data;
    }

    inline T* end() {
        return data + pos;
    }

    inline const T* begin() const {
        return data;
    }

    inline const T* end() const {
        return data + pos;
    }

    inline operator bool() const {
        return data && committed;
    }

    inline T& operator[](size_t idx) {
        return data[idx];
    }

    inline const T& operator[](size_t idx) const {
        return data[idx];
    }
};

#endif