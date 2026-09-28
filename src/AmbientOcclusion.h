#ifndef AMBIENTOCCLUSION_H_INCLUDED
#define AMBIENTOCCLUSION_H_INCLUDED

#pragma once

#include <BindingRegistry.h>
#include <rendering/GLTexture.h>
#include <shader/ShaderManager.h>

struct AmbientOcclusion {
    GLTexture aoTexture;
    uint32_t aoBinding;

    AmbientOcclusion(uvec2 size) : aoTexture(GLTexture::createTexture2D(size.x, size.y, GL_R16F)), aoBinding(BindingRegistry::allocateTextureBinding()) {
        ShaderManager::setValue("AO_TEXTURE_IDX", aoBinding);
        glBindImageTexture(aoBinding, aoTexture.ID, 0, false, 0, GL_READ_WRITE, aoTexture.format);
    }

    void setSize(uvec2 newSize) {
        if (newSize.x != aoTexture.width || newSize.y != aoTexture.height) {
            aoTexture.destroy();
            aoTexture = GLTexture::createTexture2D(newSize.x, newSize.y, GL_R16F);
            glBindImageTexture(aoBinding, aoTexture.ID, 0, false, 0, GL_READ_WRITE, aoTexture.format);
        }
    }
};

#endif