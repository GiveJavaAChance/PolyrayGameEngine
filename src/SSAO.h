#ifndef SSAO_H_INCLUDED
#define SSAO_H_INCLUDED

#pragma once

#include <cstdint>
#include <random>

#include <BindingRegistry.h>
#include <prvl.h>
#include <rendering/GLTexture.h>
#include <shader/ShaderManager.h>

enum SSAOResolveMode : uint32_t {
    NO_BLUR = 0u,
    BLUR = 1u
};

struct SSAO {
    GLTexture ssaoTexture;
    uint32_t ssaoBinding;
    ShaderProgram ssao;
    ShaderProgram ssaoResolve;

    SSAO(uvec2 size) : ssaoTexture(GLTexture::createTexture2D(size.x, size.y, GL_R16F)), ssaoBinding(BindingRegistry::allocateTextureBinding()) {
        ShaderManager::setValue("SSAO_IDX", ssaoBinding);
        glBindImageTexture(ssaoBinding, ssaoTexture.ID, 0, false, 0, GL_READ_WRITE, ssaoTexture.format);

        this->ssao = ShaderManager::createProgram({ShaderManager::compileShaderFile("res/shaders/SSAO.compute", GL_COMPUTE_SHADER)});
        this->ssaoResolve = ShaderManager::createProgram({ShaderManager::compileShaderFile("res/shaders/SSAOResolve.compute", GL_COMPUTE_SHADER)});

        std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
        std::default_random_engine generator;
        vec3 ssaoKernel[64u];
        for (uint32_t i = 0u; i < 64u; i++) {
            float scale = static_cast<float>(i) / 64.0f;
            scale = mix(0.1f, 1.0f, scale * scale);
            ssaoKernel[i] = normalize(prvl::vec3(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator))) * randomFloats(generator) * scale;
        }
        ssao.use();
        ssao.setUniform("samples", ssaoKernel, 64u);
    }

    void setSize(uvec2 newSize) {
        if (newSize.x != ssaoTexture.width || newSize.y != ssaoTexture.width) {
            ssaoTexture.destroy();
            ssaoTexture = GLTexture::createTexture2D(newSize.x, newSize.y, GL_R16F);
            glBindImageTexture(ssaoBinding, ssaoTexture.ID, 0, false, 0, GL_READ_WRITE, ssaoTexture.format);
        }
    }

    void update(const GLTexture& depthTexture) {
        glBindTextureUnit(0, depthTexture.ID);
        uint32_t groupsX = (ssaoTexture.width + 7u) >> 3u;
        uint32_t groupsY = (ssaoTexture.height + 7u) >> 3u;
        ssao.use();
        ssao.dispatchCompute(groupsX, groupsY, 1u, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void resolve(SSAOResolveMode resolveMode) {
        uint32_t groupsX = (ssaoTexture.width + 7u) >> 3u;
        uint32_t groupsY = (ssaoTexture.height + 7u) >> 3u;
        ssaoResolve.use();
        ssaoResolve.setUniform("mode", static_cast<uint32_t>(resolveMode));
        ssaoResolve.dispatchCompute(groupsX, groupsY, 1u, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
};

#endif