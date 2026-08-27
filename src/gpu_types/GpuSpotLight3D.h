#ifndef GPUSPOTLIGHT3D_H_INCLUDED
#define GPUSPOTLIGHT3D_H_INCLUDED

#pragma once

#include <cstdint>

#include <prvl.h>

struct GpuSpotLight3D {
    vec3 color;
    float strength;
    vec3 pos;
    float distanceAttenuation;
    vec3 dir;
    float spotCosAngle;
    uint32_t shadowIdx;
    uint32_t __padding__[3u];
};

#endif