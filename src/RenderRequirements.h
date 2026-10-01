#ifndef RENDERREQUIREMENTS_H_INCLUDED
#define RENDERREQUIREMENTS_H_INCLUDED

#pragma once

#include <cstdint>

#undef OPAQUE

enum AlphaMode : uint8_t {
    OPAQUE = 0u,
    MASKED = 1u,
    BLENDED = 2u
};

template <typename T>
struct Optional {
    T value;
    bool hasValue;

    Optional() : hasValue(false) {
    }

    Optional(T val) : value(val), hasValue(true) {
    }

    constexpr operator bool() const {
        return hasValue;
    }
};

struct RenderRequirements {
    Optional<AlphaMode> alphaMode;
};

#endif