#ifndef VIEWPORT_H_INCLUDED
#define VIEWPORT_H_INCLUDED

#pragma once

#include <cstdint>

#include <prvl.h>

#include <rendering/GLFramebuffer.h>
#include <rendering/ShaderBuffer.h>

struct Viewport {
    uvec2 size;
    GLenum colorFormat;
    GLenum depthFormat;
    GLFramebuffer fbo;

    Viewport(uvec2 s, GLenum colorFormat = GL_RGBA8, GLenum depthFormat = GL_DEPTH_COMPONENT32) : size(s), colorFormat(colorFormat), depthFormat(depthFormat), fbo(size.x, size.y, colorFormat, depthFormat) {
    }

    Viewport() : Viewport(prvl::uvec2(512u)) {
    }

    void setSize(uvec2 newSize) {
        if (newSize.x != size.x || newSize.y != size.y) {
            size = newSize;
            fbo.destroy();
            fbo = GLFramebuffer(size.x, size.y, colorFormat, depthFormat);
        }
    }

    void use() {
        fbo.bind();
        glViewport(0, 0, size.x, size.y);
    }
};

#endif
