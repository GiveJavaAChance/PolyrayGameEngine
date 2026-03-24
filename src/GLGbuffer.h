#ifndef GLGBUFFER_H_INCLUDED
#define GLGBUFFER_H_INCLUDED


#pragma once

#include <cstdint>
#include <initializer_list>

#include "Allocator.h"
#include "base/glad/glad.h"
#include "GLTexture.h"

struct GLGbuffer {
    GLuint ID;

    GLTexture** textures = nullptr;
    uint32_t textureCount;
    GLTexture* depth = nullptr;

    GLGbuffer(const GLGbuffer&) = delete;
    GLGbuffer& operator=(const GLGbuffer&) = delete;

    GLGbuffer(std::initializer_list<GLTexture*> tex, GLTexture* depthTex = nullptr) : textureCount(tex.size()), depth(depthTex) {
        glCreateFramebuffers(1, &ID);

        if(textureCount > 0) {
            textures = alloc<GLTexture*>(textureCount);
            GLenum* drawBuffers = alloc<GLenum>(textureCount);
            uint32_t i = 0u;
            for(GLTexture* t : tex) {
                GLenum attachment = GL_COLOR_ATTACHMENT0 + i;
                glNamedFramebufferTexture(ID, attachment, t->ID, 0);
                drawBuffers[i] = attachment;
                textures[i] = t;
                i++;
            }
            glNamedFramebufferDrawBuffers(ID, textureCount, drawBuffers);
            free(drawBuffers);
        }
        if(depthTex) {
            glNamedFramebufferTexture(ID, GL_DEPTH_ATTACHMENT, depthTex->ID, 0);
        }
        if (glCheckNamedFramebufferStatus(ID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer not complete!" << std::endl;
        }
    }

    GLGbuffer(GLGbuffer&& other) noexcept : ID(other.ID), textures(other.textures), textureCount(other.textureCount), depth(other.depth) {
        other.ID = 0;
        other.textures = nullptr;
        other.textureCount = 0;
        other.depth = nullptr;
    }

    GLGbuffer& operator=(GLGbuffer&& other) noexcept {
        if(this != &other) {
            free(textures);

            ID = other.ID;
            textures = other.textures;
            textureCount = other.textureCount;
            depth = other.depth;

            other.ID = 0;
            other.textures = nullptr;
            other.textureCount = 0;
            other.depth = nullptr;
        }
        return *this;
    }

    inline uint32_t width() const {
        return depth ? depth->width : textures[0]->width;
    }

    inline uint32_t height() const {
        return depth ? depth->height : textures[0]->height;
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
