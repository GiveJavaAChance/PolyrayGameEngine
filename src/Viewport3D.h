#ifndef VIEWPORT3D_H_INCLUDED
#define VIEWPORT3D_H_INCLUDED

#pragma once

#include <cstdint>

#include <prvl.h>

#include <Camera3D.h>
#include <rendering/GLFramebuffer.h>
#include <rendering/ShaderBuffer.h>

#include <World.h>
#include <serial/Serial.h>

struct Viewport3D {
    uvec2 size;
    GLFramebuffer fbo;
    ComponentRef<Camera3D> cam;
    bool dirty;

    Viewport3D(uvec2 s, GLenum colorFormat = GL_RGBA8, GLenum depthFormat = GL_DEPTH_COMPONENT32) : size(s), fbo(size.x, size.y, colorFormat, depthFormat), cam(UINT32_MAX), dirty(true) {
    }

    Viewport3D() : Viewport3D(prvl::uvec2(512u)) {
    }

    void setSize(const uvec2& newSize) {
        if (newSize.x != size.x || newSize.y != size.y) {
            size = newSize;
            dirty = true;
        }
    }

    bool use(const ShaderBuffer& cameraBuffer, ECS* ecs) {
        if (!ecs->isComponentValid<Camera3D>(cam.ID)) {
            return false;
        }
        Camera3D* c = ecs->getPtr(cam);
        if (dirty) {
            dirty = false;
            fbo.destroy();
            fbo = GLFramebuffer(size.x, size.y);
            c->dirty = true;
        }
        c->update(size.x, size.y);
        c->upload(cameraBuffer);
        fbo.bind();
        glViewport(0, 0, size.x, size.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        return true;
    }
};

template <>
struct Serial<Viewport3D> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        Viewport3D* viewport = world->ecs.getPtr<Viewport3D>(componentID);
        output.write(viewport->size);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        e.addComponent(Viewport3D{input.read<uvec2>()});
    }
};

template <>
struct ExportInfo<Viewport3D> {
    constexpr static Export __export__[] = {
        {offsetof(Viewport3D, size), EXPORT_UVEC2, "Size"},
        {offsetof(Viewport3D, fbo.color), EXPORT_TEXTURE, "Preview"},
        {offsetof(Viewport3D, dirty), EXPORT_DIRTY_FLAG, ""},
    };
};

#endif