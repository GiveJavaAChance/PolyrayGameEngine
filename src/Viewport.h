#ifndef VIEWPORT_H_INCLUDED
#define VIEWPORT_H_INCLUDED

#pragma once

#include <cstdint>

#include <prvl.h>

#include <rendering/GLFramebuffer.h>
#include <rendering/ShaderBuffer.h>

struct Viewport {
    GLenum colorFormat;
    GLenum depthFormat;
    uint32_t MSAASamples;
    GLFramebuffer fbo;

    Viewport(uvec2 size, GLenum colorFormat = GL_RGBA8, GLenum depthFormat = GL_DEPTH_COMPONENT32, uint32_t MSAASamples = 1u) : colorFormat(colorFormat), depthFormat(depthFormat), MSAASamples(MSAASamples), fbo(size.x, size.y, colorFormat, depthFormat, MSAASamples) {
    }

    Viewport() : Viewport(prvl::uvec2(512u)) {
    }

    void setSize(uvec2 newSize) {
        if (newSize.x != fbo.width() || newSize.y != fbo.height()) {
            fbo.destroy();
            fbo = GLFramebuffer(newSize.x, newSize.y, colorFormat, depthFormat, MSAASamples);
        }
    }

    inline uvec2 size() const {
        return prvl::uvec2(fbo.width(), fbo.height());
    }

    inline void use() {
        fbo.bind();
    }
};

#endif
