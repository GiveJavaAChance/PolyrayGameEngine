#ifndef MEMSPY_H_INCLUDED
#define MEMSPY_H_INCLUDED

#pragma once

#ifdef USE_MEMSPY

#include <cstdlib>
#include <new>
#include <unordered_map>
#include <mutex>
#include <immintrin.h>
#include <iostream>

namespace M {
    struct AllocationEntry {
        void* ptr;
        size_t sz;
    };

    extern size_t alloc_count;
    extern std::mutex alloc_mutex;
    extern size_t total_allocated;
    extern thread_local bool in_tracking;
}

void record_alloc(void* ptr, size_t sz);

void record_free(void* ptr);

size_t getTotalAllocated();

void* operator new(size_t sz);

void operator delete(void* ptr) noexcept;

void operator delete(void* ptr, size_t sz) noexcept;

void* operator new[](size_t sz);

void operator delete[](void* ptr) noexcept;

void operator delete[](void* ptr, size_t sz) noexcept;

void* _track_mm_malloc(size_t sz, size_t alignment);

void _track_mm_free(void* ptr);

#define MM_MALLOC(sz, align) _track_mm_malloc(sz, align)
#define MM_FREE(ptr)         _track_mm_free(ptr)

#endif

#endif
