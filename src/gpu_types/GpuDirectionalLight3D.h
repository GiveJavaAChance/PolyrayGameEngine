#ifndef GPUDIRECTIONALLIGHT3D_H_INCLUDED
#define GPUDIRECTIONALLIGHT3D_H_INCLUDED

#pragma once

#include <cstdint>

#include <prvl.h>

struct GpuDirectionalLight3D {
    vec3 color;
    float strength;
    vec3 dir;
    uint32_t shadowIdx;
};

#endif