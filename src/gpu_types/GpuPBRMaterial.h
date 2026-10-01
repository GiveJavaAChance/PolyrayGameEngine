#ifndef GPUPBRMATERIAL_H_INCLUDED
#define GPUPBRMATERIAL_H_INCLUDED

#pragma once

#include <prvl.h>

#include <gpu_types/GpuUvTransform.h>

struct GpuPBRMaterial {
    vec4 baseColor;

    GpuUvTransform baseColorUvTransform;
    uint64_t baseColorTexture;

    GpuUvTransform normalUvTransform;
    uint64_t normalMapTexture;

    GpuUvTransform metallicRoughnessUvTransform;
    uint64_t metallicRoughnessTexture;

    vec2 baseMetallicRougness;
    float __padding__[2u];

    vec3 F0;
    float alphaCutoff;
};

#endif