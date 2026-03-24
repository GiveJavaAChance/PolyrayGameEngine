#ifndef TIME_H_INCLUDED
#define TIME_H_INCLUDED

#pragma once
#include <cstdint>
#include <chrono>

namespace Time {
    uint64_t nanoTime() {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch());
        return duration.count();
    }
}

#endif
