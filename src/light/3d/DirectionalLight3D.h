#ifndef DIRECTIONALLIGHT3D_H_INCLUDED
#define DIRECTIONALLIGHT3D_H_INCLUDED

#include "ecs/Export.h"
#pragma once

#include <World.h>
#include <prvl.h>

struct DirectionalLight3D {
    vec3 color;
    float strength;
    bool castShadow;
    float shadowWidth;
    float shadowDepth;
    uint32_t shadowId;
    uint32_t transformID = UINT32_MAX;

    DirectionalLight3D() {
    }

    DirectionalLight3D(const vec3& color, float strength, bool castShadow = true, float shadowWidth = 100.0f, float shadowDepth = 100.0f)
        : color(color), strength(strength), castShadow(castShadow), shadowWidth(shadowWidth), shadowDepth(shadowDepth) {
    }
};

template <>
struct Serial<DirectionalLight3D> {
    constexpr static uint32_t SIZE = offsetof(DirectionalLight3D, shadowId);

    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        DirectionalLight3D* light = world->ecs.getPtr<DirectionalLight3D>(componentID);
        output.write(light, SIZE);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        DirectionalLight3D light{};
        input.read(&light, SIZE);
        e.addComponent(light);
    }
};

template <>
struct ExportInfo<DirectionalLight3D> {
    constexpr static Export __export__[] = {
        {offsetof(DirectionalLight3D, color), EXPORT_COLOR_RGB, "Color"},
        {offsetof(DirectionalLight3D, strength), EXPORT_FLOAT, "Strength"},
    };
};

#endif
