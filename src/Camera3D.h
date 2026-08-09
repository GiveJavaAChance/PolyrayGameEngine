#ifndef CAMERA3D_H_INCLUDED
#define CAMERA3D_H_INCLUDED

#pragma once

#include <cstring>

#include <Transform3D.h>
#include <World.h>
#include <ecs/ComponentRef.h>
#include <prvl.h>
#include <rendering/ShaderBuffer.h>
#include <serial/Serial.h>

struct Camera3D {
    constexpr static uint32_t DATA_SIZE = 68u;

    mat4 cameraTransform;
    mat4 inverseCameraTransform;
    mat4 projection;
    mat4 inverseProjection;
    vec3 cameraPos;

    float nearZ;
    float fov;

    bool current;
    bool dirty;

    ComponentRef<Transform3D> transformRef;

    Camera3D(float nearZ = 0.1f, float fov = 90.0f) : nearZ(nearZ), fov(fov), current(false), dirty(true), transformRef(UINT32_MAX) {
    }

    void update(float width, float height) {
        if (!dirty) {
            return;
        }
        dirty = false;
        projection = reverseZPerspectiveProjection(fov * 0.0174532925199f, width / height, nearZ);
        inverseProjection = inverse(projection);
    }

    void upload(const ShaderBuffer& cameraBuffer) {
        cameraBuffer.uploadPartialData(cameraTransform.data(), DATA_SIZE, 0u);
    }
};

template <>
struct Serial<Camera3D> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        Camera3D* cam = world->ecs.getPtr<Camera3D>(componentID);
        output.write(&cam->nearZ, 2u * sizeof(float));
        uint8_t c = cam->current;
        output.write(c);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        Camera3D cam{};
        input.read(&cam.nearZ, 2u * sizeof(float));
        cam.current = input.read<uint8_t>();
        e.addComponent(cam);
    }
};

template <>
struct ExportInfo<Camera3D> {
    constexpr static Export __export__[] = {
        {offsetof(Camera3D, nearZ), EXPORT_FLOAT, "Near Z"},
        {offsetof(Camera3D, fov), EXPORT_FLOAT, "Fov"},
        {offsetof(Camera3D, current), EXPORT_BOOL, "Current"},
        {offsetof(Camera3D, dirty), EXPORT_DIRTY_FLAG, ""},
    };
};

#endif
