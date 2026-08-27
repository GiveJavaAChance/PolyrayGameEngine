#ifndef GPUCAMERA3D_H_INCLUDED
#define GPUCAMERA3D_H_INCLUDED

#pragma once

#include <cstdint>

#include <prvl.h>

struct GpuCamera3D {
    mat4 cameraTransform;
    mat4 inverseCameraTransform;
    mat4 projection;
    mat4 inverseProjection;
    vec3 cameraPos;
    float __padding__;
};

#endif