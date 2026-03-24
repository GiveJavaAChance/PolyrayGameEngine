#ifndef GLTEXTURE_H_INCLUDED
#define GLTEXTURE_H_INCLUDED

#pragma once

#include <cstdint>

#include "base/glad/glad.h"

struct GLTexture {
    GLuint ID;
    GLenum target;
    GLenum format;

    uint32_t mipLevels;
    GLint interpolation;

    uint32_t width;
    uint32_t height;
    uint32_t depth;

    void updateFiltering() {
        GLint mode = GL_NEAREST + (interpolation & 1);
        glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, (mipLevels > 1u) ? (GL_NEAREST_MIPMAP_NEAREST + interpolation) : mode);
        glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, mode);
    }

    inline void set2DTextureData(const void* pixels, uint32_t width, uint32_t height, int x, int y, GLenum pixelFormat, GLenum pixelType) {
        glTextureSubImage2D(ID, 0, x, y, width, height, pixelFormat, pixelType, pixels);
    }

    inline void set3DTextureData(const void* pixels, uint32_t width, uint32_t height, uint32_t depth, int x, int y, int z, GLenum pixelFormat, GLenum pixelType) {
        glTextureSubImage3D(ID, 0, x, y, z, width, height, depth, pixelFormat, pixelType, pixels);
    }

    inline void set3DTextureDataLayer(const void* pixels, uint32_t width, uint32_t height, int x, int y, int z, GLenum pixelFormat, GLenum pixelType) {
        glTextureSubImage3D(ID, 0, x, y, z, width, height, 1, pixelFormat, pixelType, pixels);
    }

    void setInterpolation(bool interpolate) {
        interpolation = interpolate ? (interpolation | 1) : (interpolation & 2);
        updateFiltering();
    }

    void setWrapMode(GLint mode) {
        glTextureParameteri(ID, GL_TEXTURE_WRAP_S, mode);
        glTextureParameteri(ID, GL_TEXTURE_WRAP_T, mode);
        if(target == GL_TEXTURE_3D) {
            glTextureParameteri(ID, GL_TEXTURE_WRAP_R, mode);
        }
    }

    void setAnisotrophy(GLfloat level) {
        glTextureParameterf(ID, GL_TEXTURE_MAX_ANISOTROPY, level);
    }

    void setMipmapInterpolation(bool interpolate) {
        interpolation = interpolate ? (interpolation | 2) : (interpolation & 1);
        updateFiltering();
    }

    void generateMipmap() {
        if (mipLevels <= 1u) {
            std::cerr << "Can't generate mipmaps since mipmapping is disabled." << std::endl;
        }
        glGenerateTextureMipmap(ID);
    }

    explicit inline operator bool() const noexcept {
        return ID != 0;
    }

    inline static GLTexture createTexture2D(uint32_t width, uint32_t height, GLenum format = GL_RGBA8, uint32_t mipLevels = 1u) {
        GLTexture tex;
        tex.target = GL_TEXTURE_2D;
        tex.format = format;
        tex.mipLevels = mipLevels;
        tex.interpolation = 0;
        tex.width = width;
        tex.height = height;
        tex.depth = 1u;

        glCreateTextures(GL_TEXTURE_2D, 1, &tex.ID);
        tex.updateFiltering();
        glTextureParameteri(tex.ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(tex.ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureStorage2D(tex.ID, mipLevels, format, width, height);
        return tex;
    }

    inline static GLTexture createTextureArray(uint32_t width, uint32_t height, uint32_t layerCount, GLenum format = GL_RGBA8, uint32_t mipLevels = 1u) {
        GLTexture tex;
        tex.target = GL_TEXTURE_2D_ARRAY;
        tex.format = format;
        tex.mipLevels = mipLevels;
        tex.interpolation = 0;
        tex.width = width;
        tex.height = height;
        tex.depth = layerCount;

        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &tex.ID);
        tex.updateFiltering();
        glTextureParameteri(tex.ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(tex.ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureStorage3D(tex.ID, mipLevels, format, width, height, layerCount);
        return tex;
    }

    inline static GLTexture createTexture3D(uint32_t width, uint32_t height, uint32_t depth, GLenum format = GL_RGBA8, uint32_t mipLevels = 1u) {
        GLTexture tex;
        tex.target = GL_TEXTURE_3D;
        tex.format = format;
        tex.mipLevels = mipLevels;
        tex.interpolation = 0;
        tex.width = width;
        tex.height = height;
        tex.depth = depth;

        glCreateTextures(GL_TEXTURE_3D, 1, &tex.ID);
        tex.updateFiltering();
        glTextureParameteri(tex.ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(tex.ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(tex.ID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTextureStorage3D(tex.ID, mipLevels, format, width, height, depth);
        return tex;
    }

    inline static float getMaxAnisotropy() {
        GLfloat a;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &a);
        return a;
    }

    inline static int getMaxTextureSize() {
        GLint size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
        return size;
    }

    inline static int getMax3DTextureSize() {
        GLint size;
        glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &size);
        return size;
    }

    inline static int getMaxArrayLayers() {
        GLint layers;
        glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &layers);
        return layers;
    }
};

#endif
