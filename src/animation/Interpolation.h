#ifndef INTERPOLATION_H_INCLUDED
#define INTERPOLATION_H_INCLUDED

#pragma once

#include <cstdint>

#include <prvl.h>

enum Interpolation : uint8_t {
    STEP,
    LINEAR,
    CUBIC_SPLINE
};

#endif