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
    constexpr static uint32_t DATA_SIZE = 48u;

    mat3 cameraTransform;
    mat3 inverseCameraTransform;
    mat3 projection;
    mat3 inverseProjection;

    bool current;
    bool dirty;

    ComponentRef<Transform2D> transformRef;

    Camera2D() : current(false), dirty(true), transformRef(UINT32_MAX) {
    }

    void update(float width, float height) {
        if (!dirty) {
            return;
        }
        dirty = false;
        float xScale = 2.0f / width;
        float yScale = 2.0f / height;
        projection = prvl::mat3(prvl::vec3(xScale, 0.0f, 0.0f), prvl::vec3(0.0f, yScale, 0.0f), prvl::vec3(-1.0f, -1.0f, 1.0f));
        inverseProjection = inverse(projection);
    }

    void upload(const ShaderBuffer& cameraBuffer) {
        float data[DATA_SIZE];
        float* src = reinterpret_cast<float*>(this);
        for (uint32_t i = 0u; i < 12u; i++) {
            uint32_t srcIdx = i * 3u;
            uint32_t dstIdx = i * 4u;
            data[dstIdx] = src[srcIdx];
            data[dstIdx + 1u] = src[srcIdx + 1u];
            data[dstIdx + 2u] = src[srcIdx + 2u];
            data[dstIdx + 3u] = 0.0f;
        }
        cameraBuffer.uploadPartialData(data, DATA_SIZE, 0u);
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
        {offsetof(Camera2D, dirty), EXPORT_DIRTY_FLAG, ""},
    };
};

#endif