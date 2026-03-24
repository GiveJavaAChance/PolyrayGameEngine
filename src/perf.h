#ifndef PERF_H_INCLUDED
#define PERF_H_INCLUDED

#pragma once

#include <cstdint>

static inline uint64_t rdtsc() {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi) : : );
    return (uint64_t(hi) << 32) | lo;
}

#endif
