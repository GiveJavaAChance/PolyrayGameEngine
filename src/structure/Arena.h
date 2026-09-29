#ifndef ARENA_H_INCLUDED
#define ARENA_H_INCLUDED

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

#include <utils/VirtualMemory.h>
#include <utils/prvl_assert.h>

struct Arena {
private:
    struct Destructor {
        void (*func)(void*);
        void* obj;
        Destructor* next;
    };

    VirtualMemory::MemoryInfo info;

    Destructor* destructors = nullptr;

    static constexpr bool isPow2(size_t size) {
        return size && !(size & (size - 1));
    }

    static constexpr size_t roundToPow2Size(size_t minimumSize, size_t pow2Size) {
        return (minimumSize + pow2Size - 1ull) & ~(pow2Size - 1ull);
    }

    void runDestructorsDownTo(size_t m) {
        while (destructors && reinterpret_cast<uint8_t*>(destructors) >= base + m) {
            Destructor* d = destructors;
            destructors = d->next;
            d->func(d->obj);
        }
    }

    void destroy() {
        if (base) {
            runDestructorsDownTo(0ull);
            VirtualMemory::release(base);
        }
        base = nullptr;
        reserved = 0ull;
        committed = 0ull;
        offset = 0ull;
    }

public:
    uint8_t* base = nullptr;
    size_t reserved = 0ull;
    size_t committed = 0ull;
    size_t offset = 0ull;

    explicit Arena(size_t reserveSize = 1ull << 30u) : info(VirtualMemory::memoryInfo()) {
        reserved = roundToPow2Size(reserveSize, info.allocationGranularity);
        base = static_cast<uint8_t*>(VirtualMemory::reserve(reserved));
        if (!base) {
            reserved = 0ull;
        }
        committed = 0ull;
        offset = 0ull;
    }

    ~Arena() {
        destroy();
    }

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    Arena(Arena&& other) noexcept
        : info(VirtualMemory::memoryInfo()), destructors(other.destructors), base(other.base),
          reserved(other.reserved), committed(other.committed), offset(other.offset) {
        other.destructors = nullptr;
        other.base = nullptr;
        other.reserved = 0ull;
        other.committed = 0ull;
        other.offset = 0ull;
    }

    Arena& operator=(Arena&& other) noexcept {
        if (this != &other) {
            destroy();
            destructors = other.destructors;
            base = other.base;
            reserved = other.reserved;
            committed = other.committed;
            offset = other.offset;
            other.destructors = nullptr;
            other.base = nullptr;
            other.reserved = 0ull;
            other.committed = 0ull;
            other.offset = 0ull;
        }
        return *this;
    }

    void* alloc(size_t size, size_t alignment) {
        PRVL_ASSERT(base, "Arena not initialised")
        PRVL_ASSERT(isPow2(alignment), "Alignment must be a power of two")

        uintptr_t start = reinterpret_cast<uintptr_t>(base);
        uintptr_t curr = start + offset;
        uintptr_t aligned = (curr + (alignment - 1ull)) & (~(uintptr_t(alignment) - 1ull));
        size_t alignedOffset = static_cast<size_t>(aligned - start);

        if (alignedOffset > reserved || size > reserved - alignedOffset) {
            return nullptr;
        }

        size_t newOffset = alignedOffset + size;

        if (newOffset > committed) {
            size_t newCommitted = roundToPow2Size(newOffset, info.pageSize);
            if (newCommitted > reserved) {
                newCommitted = reserved;
            }
            if (!VirtualMemory::commit(base + committed, newCommitted - committed)) {
                return nullptr;
            }
            committed = newCommitted;
        }

        offset = newOffset;
        return reinterpret_cast<void*>(aligned);
    }

    template <typename T>
    T* alloc(uint32_t count, uint32_t alignment = alignof(T)) {
        size_t a = alignment < alignof(T) ? alignof(T) : alignment;
        if (sizeof(T) != 0ull && size_t(count) > SIZE_MAX / sizeof(T)) {
            return nullptr;
        }
        return static_cast<T*>(alloc(size_t(count) * sizeof(T), a));
    }

    template <typename T, typename... Args>
    T& emplace(Args&&... args) {
        if constexpr (std::is_trivially_destructible_v<T>) {
            void* mem = alloc(sizeof(T), alignof(T));
            if (!mem) {
                throw std::bad_alloc();
            }
            return *::new (mem) T(std::forward<Args>(args)...);
        } else {
            Destructor* d = static_cast<Destructor*>(alloc(sizeof(Destructor), alignof(Destructor)));
            void* mem = alloc(sizeof(T), alignof(T));
            if (!d || !mem) {
                throw std::bad_alloc();
            }
            T* obj = ::new (mem) T(std::forward<Args>(args)...);
            d->func = [](void* p) { static_cast<T*>(p)->~T(); };
            d->obj = obj;
            d->next = destructors;
            destructors = d;
            return *obj;
        }
    }

    inline size_t mark() const {
        return offset;
    }

    inline void rewind(size_t m) {
        PRVL_ASSERT(m <= offset, "Mark out of range");
        runDestructorsDownTo(m);
        offset = m;
    }

    void reset(bool releasePages = false) {
        runDestructorsDownTo(0ull);
        offset = 0ull;
        if (releasePages && committed) {
            VirtualMemory::decommit(base, committed);
            committed = 0ull;
        }
    }

    inline operator bool() const {
        return base && committed;
    }
};

#endif