#ifndef GLTEXTUREMSAA_H_INCLUDED
#define GLTEXTUREMSAA_H_INCLUDED

#pragma once

#include <cstdint>

#include "base/glad/glad.h"

struct GLTextureMSAA {
    GLuint ID;

    uint32_t width;
    uint32_t height;

    GLTextureMSAA(uint32_t width, uint32_t height, GLenum format, uint32_t samples) : width(width), height(height) {
        glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &ID);
        glTextureStorage2DMultisample(ID, samples, format, width, height, GL_TRUE);
    }
};

#endif
