#ifndef DYNAMICARRAY_H_INCLUDED
#define DYNAMICARRAY_H_INCLUDED

#pragma once
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>
#include "Allocator.h"

using u32 = uint32_t;

template<typename T>
struct DynamicArray {
private:
    u32 length;
    T* ptr;
    u32 pos;

public:

    DynamicArray(uint32_t initialSize = 16u) : length(initialSize), ptr(alloc<T>(initialSize)), pos(0u) {
    }

    ~DynamicArray() {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (u32 i = 0; i < pos; ++i) {
                ptr[i].~T();
            }
        }
        free(ptr);
    }

    DynamicArray(const DynamicArray&) = delete;
    DynamicArray& operator=(const DynamicArray&) = delete;

    DynamicArray(DynamicArray&& other) noexcept : length(other.length), ptr(other.ptr), pos(other.pos) {
        other.ptr = nullptr;
        other.length = 0;
        other.pos = 0;
    }

    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if(this != &other) {
            free(ptr);
            length = other.length;
            ptr = other.ptr;
            pos = other.pos;
            other.ptr = nullptr;
            other.length = 0;
            other.pos = 0;
        }
        return *this;
    }

    // This function simply ensures that there's enough capacity and reallocates the array if it's too small
    void ensureCapacity(u32 cap, bool clear = false) {
        if (cap <= length) return;
        cap = (cap * 3u) >> 1u; // 1.5x growth rate
        T* tmp = alloc<T>(cap);
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy(tmp, ptr, length * sizeof(T));
        } else {
            for (u32 i = 0; i < pos; ++i) {
                new (tmp + i) T(std::move(ptr[i]));
                ptr[i].~T();
            }
        }
        if (clear) {
            std::memset(reinterpret_cast<void*>(tmp + length), 0, (cap - length) * sizeof(T));
        }
        free(ptr);
        ptr = tmp;
        length = cap;
    }

    // Function for emplacing one thing directly
    template<typename... Args>
    inline void emplace(Args&&... args) {
        ensureCapacity(pos + 1u);
        T* place = ptr + pos;
        new (place) T(std::forward<Args>(args)...);
        pos++;
    }

    // Function for adding one thing at a time
    inline void add(const T& e) {
        emplace(e);
    }

    // Function for adding one immediate at a time
    inline void add(T&& e) {
        emplace(std::move(e));
    }

    // Function for adding multiple things at the same time
    inline void addAll(T* p, u32 count) {
        ensureCapacity(pos + count);
        std::memcpy(ptr + pos, p, count * sizeof(T));
        pos += count;
    }

    // This function simply fills the array with uniniialized memory
    inline void reserve(u32 count, bool clear = false) {
        ensureCapacity(pos + count, clear);
        pos += count;
    }

    inline void removeEnd(u32 count) {
        pos-= count;
    }

    // Simply resets the position, and optionally clears the data
    inline void clear(bool clear = false) {
        if(clear) {
            std::memset(ptr, 0, pos * sizeof(T));
        }
        pos = 0u;
    }

    // This overloads the indexing operator to make it possible to index directly to the array
    inline T& operator[](u32 i) const {
        return ptr[i];
    }

    // A shorthand for accessing the pointer directly with some displacement instead of doing &arr[i], just doing arr + i
    inline T* operator+(u32 i) const {
        return ptr + i;
    }

    // Get the size of the array
    inline u32 size() const {
        return pos;
    }

    // Get the backing data
    inline T* data() const {
        return ptr;
    }
};

#endif
