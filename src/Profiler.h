#ifndef PROFILER_H_INCLUDED
#define PROFILER_H_INCLUDED

#pragma once

#include <cstdint>
#include <utils/perf.h>

namespace Profiler {
    struct ScopeStats {
        uint64_t cycles;
        uint32_t calls;
    };

    inline uint64_t profileStartNs = 0ull;
    inline uint64_t profileStartTsc = 0ull;
    inline uint64_t profileEndNs = 0ull;
    inline uint64_t profileEndTsc = 0ull;

    inline ScopeStats stats[1 << 20];
    inline uint32_t idCounter = 0u;

    inline const char* scopeNames[1 << 20];

    struct ProfileScope {
        uint32_t id;
        uint64_t start;

        ProfileScope(uint32_t id) : id(id), start(Time::rdtsc()) {}

        ~ProfileScope() {
            uint64_t dt = Time::rdtsc() - start;
            stats[id].cycles += dt;
            stats[id].calls++;
        }
    };

    inline void begin() {
        for(uint32_t i = 0u; i < idCounter; i++) {
            stats[i] = {};
        }
        profileStartTsc = Time::rdtsc();
        profileStartNs = Time::nanoTime();
    }

    inline double* end(double nsBudget) {
        profileEndNs = Time::nanoTime();
        profileEndTsc = Time::rdtsc();
        double nsPerTsc = static_cast<double>(profileEndNs - profileStartNs) / static_cast<double>(profileEndTsc - profileStartTsc);
        double* buffer = new double[idCounter];
        for(uint32_t i = 0u; i < idCounter; i++) {
            buffer[i] = static_cast<double>(stats[i].cycles) * nsPerTsc / nsBudget;
        }
        return buffer;
    }

    inline uint32_t getID() {
        return idCounter++;
    }
}

#define PROFILE

#ifdef PROFILE
#define PROFILE_SCOPE(name)                     \
static uint32_t PROFILE_ID = Profiler::getID(); \
Profiler::ProfileScope SCOPE(PROFILE_ID);       \
Profiler::scopeNames[PROFILE_ID] = #name;
#else
#define PROFILE_SCOPE(name)
#endif

#endif
