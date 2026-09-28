#ifndef BLOOM_H_INCLUDED
#define BLOOM_H_INCLUDED

#pragma once

#include <cstdint>

#include <BindingRegistry.h>
#include <FullscreenQuad.h>
#include <rendering/GLTexture.h>
#include <shader/ShaderManager.h>

struct Bloom {
private:
    GLenum bloomFormat;

    GLuint bloomBinding;

    ShaderProgram bloomThreshold;
    ShaderProgram bloomDownsample;
    ShaderProgram bloomUpsample;

    FullscreenQuad bloomComposite;

public:
    GLTexture bloomTexture;
    float intensity;
    float threshold;

    Bloom(uvec2 size, GLenum format)
        : bloomFormat(format), bloomBinding(BindingRegistry::allocateTextureBinding()),
          bloomComposite(ShaderManager::compileShaderFile("res/shaders/bloom/BloomComposite.frag", GL_FRAGMENT_SHADER)),
          bloomTexture(GLTexture::createTexture2D(size.x, size.y, format, -1)), intensity(1.0f), threshold(1.0f) {
        bloomTexture.setInterpolation(true);
        ShaderManager::setValue("BLOOM_IDX", bloomBinding);
        this->bloomThreshold = ShaderManager::createProgram({ShaderManager::compileShaderFile("res/shaders/bloom/BloomThreshold.compute", GL_COMPUTE_SHADER)});
        this->bloomDownsample = ShaderManager::createProgram({ShaderManager::compileShaderFile("res/shaders/bloom/BloomDownsample.compute", GL_COMPUTE_SHADER)});
        this->bloomUpsample = ShaderManager::createProgram({ShaderManager::compileShaderFile("res/shaders/bloom/BloomUpsample.compute", GL_COMPUTE_SHADER)});
        bloomComposite.quadProgram.setUniform("layers", static_cast<int32_t>(bloomTexture.mipLevels));
    }

    void setSize(uvec2 newSize) {
        if (newSize.x != bloomTexture.width || newSize.y != bloomTexture.height) {
            bloomTexture.destroy();
            bloomTexture = GLTexture::createTexture2D(newSize.x, newSize.y, bloomFormat, -1);
            bloomComposite.quadProgram.setUniform("layers", static_cast<int32_t>(bloomTexture.mipLevels));
        }
    }

    void render(GLTexture& colorTexture, GLTexture* lensDirtTexture = nullptr) {
        bloomThreshold.use();
        glBindTextureUnit(0, colorTexture.ID);
        glBindImageTexture(bloomBinding, bloomTexture.ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, bloomTexture.format);
        bloomThreshold.setUniform("threshold", threshold);
        bloomThreshold.dispatchCompute((bloomTexture.width + 7u) >> 3u, (bloomTexture.height + 7u) >> 3u, 1u, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glBindTextureUnit(0, bloomTexture.ID);
        bloomDownsample.use();
        for (uint32_t i = 1u; i < bloomTexture.mipLevels; i++) {
            bloomDownsample.setUniform("layer", static_cast<int32_t>(i - 1u));
            glBindImageTexture(bloomBinding, bloomTexture.ID, i, GL_FALSE, 0, GL_WRITE_ONLY, bloomTexture.format);
            uint32_t wid = bloomTexture.width >> i;
            uint32_t hei = bloomTexture.height >> i;
            bloomDownsample.dispatchCompute((wid + 7u) >> 3u, (hei + 7u) >> 3u, 1u, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
        bloomUpsample.use();
        for (uint32_t i = bloomTexture.mipLevels - 1u; i > 0u; i--) {
            uint32_t j = i - 1u;
            bloomUpsample.setUniform("layer", static_cast<int32_t>(i));
            glBindImageTexture(bloomBinding, bloomTexture.ID, j, GL_FALSE, 0, GL_READ_WRITE, bloomTexture.format);
            uint32_t wid = bloomTexture.width >> j;
            uint32_t hei = bloomTexture.height >> j;
            bloomUpsample.dispatchCompute((wid + 7u) >> 3u, (hei + 7u) >> 3u, 1u, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
        glBindTextureUnit(1, colorTexture.ID);
        bloomComposite.quadProgram.use();
        bloomComposite.quadProgram.setUniform("intensity", intensity);
        if (lensDirtTexture) {
            glBindTextureUnit(2, lensDirtTexture->ID);
            bloomComposite.quadProgram.setUniform("useLensDirt", 1u);
        } else {
            bloomComposite.quadProgram.setUniform("useLensDirt", 0u);
        }
        bloomComposite.render();
    }
};

#endif
