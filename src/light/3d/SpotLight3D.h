#ifndef SPOTLIGHT3D_H_INCLUDED
#define SPOTLIGHT3D_H_INCLUDED

#pragma once

#include <World.h>
#include <prvl.h>

struct SpotLight3D {
    vec3 color;
    float strength;
    float distanceAttenuation;
    float spotAngle;
    bool castShadow;
    float shadowNearZ;
    uint32_t shadowId;
    uint32_t transformID = UINT32_MAX;

    SpotLight3D() {
    }

    SpotLight3D(const vec3& color, float strength, float distanceAttenuation, float spotAngle, bool castShadow = true, float shadowNearZ = 0.1f)
        : color(color), strength(strength), distanceAttenuation(distanceAttenuation), spotAngle(spotAngle), castShadow(castShadow), shadowNearZ(shadowNearZ) {
    }
};

template <>
struct Serial<SpotLight3D> {
    constexpr static uint32_t SIZE = offsetof(SpotLight3D, shadowId);

    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        SpotLight3D* light = world->ecs.getPtr<SpotLight3D>(componentID);
        output.write(light, SIZE);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        SpotLight3D light{};
        input.read(&light, SIZE);
        e.addComponent(light);
    }
};

template <>
struct ExportInfo<SpotLight3D> {
    constexpr static Export __export__[] = {
        {offsetof(SpotLight3D, color), EXPORT_COLOR_RGB, "Color"},
        {offsetof(SpotLight3D, strength), EXPORT_FLOAT, "Strength"},
        {offsetof(SpotLight3D, distanceAttenuation), EXPORT_FLOAT, "Distance Attenuation"},
    };
};

#endif
