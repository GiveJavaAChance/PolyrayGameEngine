#ifndef MATERIALTYPE_H_INCLUDED
#define MATERIALTYPE_H_INCLUDED

#pragma once

#include <RenderMode.h>
#include <rendering/ShaderBuffer.h>
#include <shader/ShaderManager.h>
#include <shader/ShaderProgram.h>

struct MaterialType {
    ShaderProgram forward;
    ShaderProgram deferred;
    ShaderProgram depth;
    VertexLayoutInfo vertexLayout;

    MaterialType() {
    }

    MaterialType(const VertexLayoutInfo& vertexLayout, const ShaderProgram& forward, const ShaderProgram& deferred = {}, const ShaderProgram& depth = {})
        : forward(forward), deferred(deferred), depth(depth), vertexLayout(vertexLayout) {
    }

    inline void use(RenderMode renderMode) {
        switch (renderMode) {
            case RenderMode::FORWARD: {
                forward.use();
                break;
            }
            case RenderMode::DEFERRED: {
                deferred.use();
                break;
            }
            case RenderMode::DEPTH: {
                depth.use();
                break;
            }
        }
    }

    inline static MaterialType staticPBR() {
        uint32_t vertStaticPBR      = ShaderManager::compileShaderFile("res/shaders/3d/StaticPBR.vert", GL_VERTEX_SHADER);
        uint32_t vertStaticPBRDepth = ShaderManager::compileShaderFile("res/shaders/3d/StaticPBRDepth.vert", GL_VERTEX_SHADER);
        uint32_t fragPBR            = ShaderManager::compileShaderFile("res/shaders/3d/PBR.frag", GL_FRAGMENT_SHADER);
        uint32_t fragPBRDeferred    = ShaderManager::compileShaderFile("res/shaders/3d/PBRDeferred.frag", GL_FRAGMENT_SHADER);
        uint32_t fragPBRDepth       = ShaderManager::compileShaderFile("res/shaders/3d/PBRDepth.frag", GL_FRAGMENT_SHADER);

        ShaderProgram forwardShader = ShaderManager::createProgram({vertStaticPBR, fragPBR});
        return MaterialType{
            ShaderManager::getVertexLayout(forwardShader),
            forwardShader,
            ShaderManager::createProgram({vertStaticPBR, fragPBRDeferred}),
            ShaderManager::createProgram({vertStaticPBRDepth, fragPBRDepth})};
    }

    inline static MaterialType skinnedPBR() {
        uint32_t vertSkinnedPBR      = ShaderManager::compileShaderFile("res/shaders/3d/SkinnedPBR.vert", GL_VERTEX_SHADER);
        uint32_t vertSkinnedPBRDepth = ShaderManager::compileShaderFile("res/shaders/3d/SkinnedPBRDepth.vert", GL_VERTEX_SHADER);
        uint32_t fragPBR             = ShaderManager::compileShaderFile("res/shaders/3d/PBR.frag", GL_FRAGMENT_SHADER);
        uint32_t fragPBRDeferred     = ShaderManager::compileShaderFile("res/shaders/3d/PBRDeferred.frag", GL_FRAGMENT_SHADER);
        uint32_t fragPBRDepth        = ShaderManager::compileShaderFile("res/shaders/3d/PBRDepth.frag", GL_FRAGMENT_SHADER);

        ShaderProgram forwardShader = ShaderManager::createProgram({vertSkinnedPBR, fragPBR});
        return MaterialType{
            ShaderManager::getVertexLayout(forwardShader),
            forwardShader,
            ShaderManager::createProgram({vertSkinnedPBR, fragPBRDeferred}),
            ShaderManager::createProgram({vertSkinnedPBRDepth, fragPBRDepth})};
    }
};

#endif