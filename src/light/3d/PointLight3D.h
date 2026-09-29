#ifndef POINTLIGHT3D_H_INCLUDED
#define POINTLIGHT3D_H_INCLUDED

#pragma once

#include <World.h>
#include <prvl.h>

struct PointLight3D {
    vec3 color;
    float strength;
    float distanceAttenuation;
    bool castShadow;
    float shadowNearZ;
    uint32_t shadowId[6u];
    uint32_t transformID = UINT32_MAX;

    PointLight3D() {
    }

    PointLight3D(const vec3& color, float strength, float distanceAttenuation, bool castShadow = true, float shadowNearZ = 0.1f)
        : color(color), strength(strength), distanceAttenuation(distanceAttenuation), castShadow(castShadow), shadowNearZ(shadowNearZ) {
    }
};

template <>
struct Serial<PointLight3D> {
    constexpr static uint32_t SIZE = offsetof(PointLight3D, shadowId);

    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        PointLight3D* light = world->ecs.getPtr<PointLight3D>(componentID);
        output.write(light, SIZE);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        PointLight3D light{};
        input.read(&light, SIZE);
        e.addComponent(light);
    }
};

template <>
struct ExportInfo<PointLight3D> {
    constexpr static Export __export__[] = {
        {offsetof(PointLight3D, color), EXPORT_COLOR_RGB, "Color"},
        {offsetof(PointLight3D, strength), EXPORT_FLOAT, "Strength"},
        {offsetof(PointLight3D, distanceAttenuation), EXPORT_FLOAT, "Distance Attenuation"},
    };
};

#endif
