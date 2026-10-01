#ifndef GLTFLOADER_H_INCLUDED
#define GLTFLOADER_H_INCLUDED

#pragma once

#include <cstdint>

#include <RenderGroupInfo.h>
#include <light/3d/DirectionalLight3D.h>
#include <light/3d/PointLight3D.h>
#include <light/3d/SpotLight3D.h>
#include <structure/DynamicArray.h>

struct ResourcePath;
struct World;

struct GltfLight {
    enum Type : uint8_t {
        DIRECTIONAL,
        SPOT,
        POINT
    };

    Type type;
    union {
        DirectionalLight3D directional;
        SpotLight3D spot;
        PointLight3D point;
    };
};

struct GltfLoader {
    static uint32_t load(const ResourcePath& res, World* world, uint32_t fromNode, const DynamicArray<RenderGroupInfo>& renderGroups = {
                                                                                       RenderGroupInfo::staticPBR(),
                                                                                       RenderGroupInfo::skinnedPBR(),
                                                                                       RenderGroupInfo::maskedStaticPBR(),
                                                                                       RenderGroupInfo::maskedStaticPBR(),
                                                                                   });
};

#endif