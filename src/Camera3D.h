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
    } perspectiveInfo;
    struct {
        float width;
        float nearZ;
        float farZ;
    } orthographicInfo;

    bool current;

    ComponentRef<Transform3D> transformRef;

    Camera3D() : projectionMode(ProjectionMode::PERSPECTIVE), current(false), transformRef(UINT32_MAX) {
    }

    mat4 getProjection(vec2 size) {
        float aspectRatio = size.x / size.y;
        if (projectionMode == ProjectionMode::PERSPECTIVE) {
            return reverseZPerspectiveProjection(perspectiveInfo.fov * 0.0174532925199f, aspectRatio, perspectiveInfo.nearZ);
        }
        return orthographicProjection(orthographicInfo.width, aspectRatio, orthographicInfo.nearZ, orthographicInfo.farZ);
    }

    static Camera3D perspective(float nearZ = 0.1f, float fov = 90.0f) {
        Camera3D cam{};
        cam.perspectiveInfo.nearZ = nearZ;
        cam.perspectiveInfo.fov = fov;
        return cam;
    }

    static Camera3D orthographic(float width, float nearZ = 0.1f, float farZ = 10.0f) {
        Camera3D cam{};
        cam.projectionMode = ProjectionMode::ORTHOGRAPHIC;
        cam.orthographicInfo.width = width;
        cam.orthographicInfo.nearZ = nearZ;
        cam.orthographicInfo.farZ = farZ;
        return cam;
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
        {offsetof(Camera3D, perspectiveInfo.nearZ), EXPORT_FLOAT, "Perspective Near Z"},
        {offsetof(Camera3D, perspectiveInfo.fov), EXPORT_FLOAT, "Perspective Fov"},
        {offsetof(Camera3D, orthographicInfo.width), EXPORT_FLOAT, "Orthographic Width"},
        {offsetof(Camera3D, orthographicInfo.nearZ), EXPORT_FLOAT, "Orthographic Near Z"},
        {offsetof(Camera3D, orthographicInfo.farZ), EXPORT_FLOAT, "Orthographic Far Z"},
        {offsetof(Camera3D, current), EXPORT_BOOL, "Current"},
    };
};

#endif
