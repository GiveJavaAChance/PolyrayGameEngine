#ifndef ALLOCATOR_H_INCLUDED
#define ALLOCATOR_H_INCLUDED

#pragma once
#include <cstdint>
#include <immintrin.h>
//#include "memspy.h"

template<typename T>
T* alloc(uint32_t length) {
    return static_cast<T*>(_mm_malloc(length * sizeof(T), 32));
}

template<typename T>
void free(T* ptr) {
    _mm_free(ptr);
}

#endif
