#ifndef GPUPOINTLIGHT3D_H_INCLUDED
#define GPUPOINTLIGHT3D_H_INCLUDED

#pragma once

#include <cstdint>

#include <prvl.h>

struct GpuPointLight3D {
    vec3 color;
    float strength;
    vec3 pos;
    float distanceAttenuation;
    uint32_t shadowIdx[6u];
    uint32_t __padding__[2u];
};

#endif