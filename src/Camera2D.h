#ifndef CAMERA2D_H_INCLUDED
#define CAMERA2D_H_INCLUDED

#pragma once

#include <Transform2D.h>
#include <World.h>
#include <ecs/ComponentRef.h>
#include <prvl.h>
#include <rendering/ShaderBuffer.h>
#include <serial/Serial.h>

struct Camera2D {
    mat3 cameraTransform;
    mat3 inverseCameraTransform;

    bool current;

    ComponentRef<Transform2D> transformRef;

    Camera2D() : current(false), transformRef(UINT32_MAX) {
    }

    mat3 getProjection(vec2 size) {
        vec2 scale = 2.0f / size;
        return prvl::mat3(prvl::vec3(scale.x, 0.0f, 0.0f), prvl::vec3(0.0f, scale.y, 0.0f), prvl::vec3(-1.0f, -1.0f, 1.0f));
    }
};

template <>
struct Serial<Camera2D> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        Camera2D* cam = world->ecs.getPtr<Camera2D>(componentID);
        uint8_t c = cam->current;
        output.write(c);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        Camera2D cam{};
        cam.current = input.read<uint8_t>();
        e.addComponent(cam);
    }
};

template <>
struct ExportInfo<Camera2D> {
    constexpr static Export __export__[] = {
        {offsetof(Camera2D, current), EXPORT_BOOL, "Current"},
    };
};

#endif
