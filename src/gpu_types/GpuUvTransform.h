#ifndef GPUUVTRANSFORM_H_INCLUDED
#define GPUUVTRANSFORM_H_INCLUDED

#pragma once

#include <prvl.h>

struct GpuUvTransform {
    vec2 offset = {0.0f, 0.0f};
    vec2 scale = {1.0f, 1.0f};
    float rotation = 0.0f;
    float __padding__;
};

#endif