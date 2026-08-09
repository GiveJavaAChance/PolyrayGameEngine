#ifndef VIEWPORT2D_H_INCLUDED
#define VIEWPORT2D_H_INCLUDED

#pragma once

#include <cstdint>

#include <prvl.h>

#include <Camera2D.h>
#include <rendering/GLFramebuffer.h>
#include <rendering/ShaderBuffer.h>

#include <World.h>
#include <serial/Serial.h>

struct Viewport2D {
    uvec2 size;
    GLFramebuffer fbo;
    ComponentRef<Camera2D> cam;
    bool dirty;

    Viewport2D(uvec2 s, GLenum colorFormat = GL_RGBA8, GLenum depthFormat = GL_DEPTH_COMPONENT32) : size(s), fbo(size.x, size.y, colorFormat, depthFormat), cam(UINT32_MAX), dirty(true) {
    }

    Viewport2D() : Viewport2D(prvl::uvec2(512u)) {
    }

    void setSize(const uvec2& newSize) {
        if (newSize.x != size.x || newSize.y != size.y) {
            size = newSize;
            dirty = true;
        }
    }

    bool use(const ShaderBuffer& cameraBuffer, ECS* ecs) {
        if (!ecs->isComponentValid<Camera2D>(cam.ID)) {
            return false;
        }
        Camera2D* c = ecs->getPtr(cam);
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
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        return true;
    }
};

template <>
struct Serial<Viewport2D> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        Viewport2D* viewport = world->ecs.getPtr<Viewport2D>(componentID);
        output.write(viewport->size);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        e.addComponent(Viewport2D{input.read<uvec2>()});
    }
};

template <>
struct ExportInfo<Viewport2D> {
    constexpr static Export __export__[] = {
        {offsetof(Viewport2D, size), EXPORT_UVEC2, "Size"},
        {offsetof(Viewport2D, dirty), EXPORT_DIRTY_FLAG, ""},
    };
};

#endif