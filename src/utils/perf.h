#ifndef PERF_H_INCLUDED
#define PERF_H_INCLUDED

#pragma once

#include <cstdint>
#include <chrono>

namespace Time {
    inline uint64_t rdtsc() {
        uint32_t lo, hi;
        __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi) : : );
        return (uint64_t(hi) << 32) | lo;
    }

    inline uint64_t nanoTime() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
}

#endif
