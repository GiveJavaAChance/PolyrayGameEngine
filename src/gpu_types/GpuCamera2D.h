#ifndef GPUCAMERA2D_H_INCLUDED
#define GPUCAMERA2D_H_INCLUDED

#pragma once

#include <cstdint>

#include <prvl.h>

struct GpuCamera2D {
    mat3x4 cameraTransform;
    mat3x4 inverseCameraTransform;
    mat3x4 projection;
    mat3x4 inverseProjection;
};

#endif