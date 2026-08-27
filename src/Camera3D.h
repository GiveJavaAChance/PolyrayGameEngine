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

#include <gpu_types/GpuCamera3D.h>

enum ProjectionMode : uint8_t {
    PERSPECTIVE,
    ORTHOGRAPHIC
};

struct Camera3D {
    mat4 cameraTransform;
    mat4 inverseCameraTransform;
    vec3 cameraPos;

    ProjectionMode projectionMode;
    struct {
        float nearZ;
        float fov;
    } perspective;
    struct {
        float width;
        float nearZ;
        float farZ;
    } orthographic;

    bool current;

    ComponentRef<Transform3D> transformRef;

    Camera3D(float nearZ = 0.1f, float fov = 90.0f) : projectionMode(ProjectionMode::PERSPECTIVE), perspective(nearZ, fov), current(false), transformRef(UINT32_MAX) {
    }

    Camera3D(float width, float nearZ = 0.1f, float farZ = 10.0f) : projectionMode(ProjectionMode::ORTHOGRAPHIC), orthographic(width, nearZ, farZ), current(false), transformRef(UINT32_MAX) {
    }

    mat4 getProjection(vec2 size) {
        float aspectRatio = size.x / size.y;
        if (projectionMode == ProjectionMode::PERSPECTIVE) {
            return reverseZPerspectiveProjection(perspective.fov * 0.0174532925199f, aspectRatio, perspective.nearZ);
        }
        return orthographicProjection(orthographic.width, aspectRatio, orthographic.nearZ, orthographic.farZ);
    }
};

template <>
struct Serial<Camera3D> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        Camera3D* cam = world->ecs.getPtr<Camera3D>(componentID);
        output.write(&cam->projectionMode, 24u);
        uint8_t c = cam->current;
        output.write(c);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        Camera3D cam{};
        input.read(&cam.projectionMode, 24u);
        cam.current = input.read<uint8_t>();
        e.addComponent(cam);
    }
};

template <>
struct ExportInfo<Camera3D> {
    constexpr static Export __export__[] = {
        {offsetof(Camera3D, projectionMode), EXPORT_BOOL, "Orthographic"},
        {offsetof(Camera3D, perspective.nearZ), EXPORT_FLOAT, "Perspective Near Z"},
        {offsetof(Camera3D, perspective.fov), EXPORT_FLOAT, "Perspective Fov"},
        {offsetof(Camera3D, orthographic.width), EXPORT_FLOAT, "Orthographic Width"},
        {offsetof(Camera3D, orthographic.nearZ), EXPORT_FLOAT, "Orthographic Near Z"},
        {offsetof(Camera3D, orthographic.farZ), EXPORT_FLOAT, "Orthographic Far Z"},
        {offsetof(Camera3D, current), EXPORT_BOOL, "Current"},
    };
};

#endif
