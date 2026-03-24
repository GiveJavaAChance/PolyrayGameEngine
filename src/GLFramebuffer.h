#ifndef GLFRAMEBUFFER_H_INCLUDED
#define GLFRAMEBUFFER_H_INCLUDED

#pragma once

#include <cstdint>

#include "base/glad/glad.h"
#include "GLTexture.h"

struct GLFramebuffer {
    GLuint ID;

    GLTexture color;
    GLTexture depth;

    GLFramebuffer(uint32_t width, uint32_t height, GLenum colorFormat = GL_RGBA8, GLenum depthFormat = GL_DEPTH_COMPONENT32) {
        glCreateFramebuffers(1, &ID);

        color = GLTexture::createTexture2D(width, height, colorFormat);
        depth = GLTexture::createTexture2D(width, height, depthFormat);

        glNamedFramebufferTexture(ID, GL_COLOR_ATTACHMENT0, color.ID, 0);
        glNamedFramebufferTexture(ID, GL_DEPTH_ATTACHMENT, depth.ID, 0);
        GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
        glNamedFramebufferDrawBuffers(ID, 1, drawBuffers);
        if (glCheckNamedFramebufferStatus(ID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer not complete!" << std::endl;
        }
    }

    inline uint32_t width() const {
        return color.width;
    }

    inline uint32_t height() const {
        return color.height;
    }

    void bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
    }

    static void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    explicit inline operator bool() const noexcept {
        return ID != 0;
    }
};

#endif
