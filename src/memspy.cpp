#ifdef USE_MEMSPY

#include "memspy.h"
#include "Game Library/DynamicArray.h"

namespace M {
    inline size_t alloc_count = 0;
    inline std::mutex alloc_mutex;
    inline size_t total_allocated = 0;
    inline thread_local bool in_tracking = false;
}

using namespace M;

DynamicArray<AllocationEntry>& getAllocations() {
    static DynamicArray<AllocationEntry> allocations_instance;
    return allocations_instance;
}

void record_alloc(void* ptr, size_t sz) {
    if (!ptr) return;
    if (in_tracking) return;
    in_tracking = true;
    std::lock_guard<std::mutex> lock(alloc_mutex);
    getAllocations().add({ptr, sz});
    alloc_count++;
    total_allocated += sz;
    in_tracking = false;
}

void record_free(void* ptr) {
    if (!ptr) return;
    if (in_tracking) return;
    in_tracking = true;
    std::lock_guard<std::mutex> lock(alloc_mutex);
    DynamicArray<AllocationEntry>& allocs = getAllocations();
    for (size_t i = 0; i < alloc_count; ++i) {
        if (allocs[i].ptr == ptr) {
            total_allocated -= allocs[i].sz;
            allocs[i] = allocs[--alloc_count];
            return;
        }
    }
    in_tracking = false;
}

size_t getTotalAllocated() {
    std::lock_guard<std::mutex> lock(alloc_mutex);
    return total_allocated;
}

void* operator new(size_t sz) {
    void* ptr = std::malloc(sz);
    if (!ptr) throw std::bad_alloc();
    record_alloc(ptr, sz);
    return ptr;
}

void operator delete(void* ptr) noexcept {
    record_free(ptr);
    std::free(ptr);
}

void operator delete(void* ptr, size_t sz) noexcept {
    record_free(ptr);
    std::free(ptr);
}

void* operator new[](size_t sz) {
    void* ptr = std::malloc(sz);
    if (!ptr) throw std::bad_alloc();
    record_alloc(ptr, sz);
    return ptr;
}

void operator delete[](void* ptr) noexcept {
    record_free(ptr);
    std::free(ptr);
}

void operator delete[](void* ptr, size_t sz) noexcept {
    record_free(ptr);
    std::free(ptr);
}

void* _track_mm_malloc(size_t sz, size_t alignment) {
    void* ptr = _mm_malloc(sz, alignment);
    record_alloc(ptr, sz);
    return ptr;
}

void _track_mm_free(void* ptr) {
    record_free(ptr);
    _mm_free(ptr);
}

#endif
