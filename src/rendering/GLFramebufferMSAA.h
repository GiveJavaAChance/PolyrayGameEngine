#ifndef GLFRAMEBUFFERMSAA_H_INCLUDED
#define GLFRAMEBUFFERMSAA_H_INCLUDED

#pragma once

#include <cstdint>
#include <iostream>

#include <base/glad/glad.h>
#include <rendering/GLTextureMSAA.h>

struct GLFramebufferMSAA {
    GLuint ID;

    GLTextureMSAA color;
    GLTextureMSAA depth;

    GLFramebufferMSAA(uint32_t width, uint32_t height, uint32_t samples, GLenum colorFormat = GL_RGBA8, GLenum depthFormat = GL_DEPTH_COMPONENT32) : color(width, height, colorFormat, samples), depth(width, height, depthFormat, samples) {
        glCreateFramebuffers(1, &ID);

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
