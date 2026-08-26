#ifndef MATERIAL_H_INCLUDED
#define MATERIAL_H_INCLUDED

#pragma once

#include <RenderMode.h>
#include <prvl.h>
#include <shader/ShaderManager.h>
#include <shader/ShaderProgram.h>
#include <structure/DynamicArray.h>

struct Material {
    ShaderProgram forward;
    ShaderProgram deferred;
    ShaderProgram depth;

    DynamicArray<GLTexture> textures;

    Material() {
    }

    Material(const ShaderProgram& forward, const ShaderProgram& deferred, const ShaderProgram& depth) : forward(forward), deferred(deferred), depth(depth) {
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
        for (uint32_t i = 0u; i < textures.size(); i++) {
            glBindTextureUnit(i, textures[i].ID);
        }
    }

    inline static Material defaultTexture3D(const GLTexture& albedo, const GLTexture& normal, const GLTexture& roughnessMetallic, const vec3& F0) {
        Material mat;

        uint32_t vert = ShaderManager::compileShaderFile("res/shaders/3d/Texture3D.vert", GL_VERTEX_SHADER);
        uint32_t frag = ShaderManager::compileShaderFile("res/shaders/3d/Texture3D.frag", GL_FRAGMENT_SHADER);
        mat.forward = ShaderManager::createProgram({vert, frag});

        mat.forward.use();
        mat.forward.setUniform("F0", F0);

        frag = ShaderManager::compileShaderFile("res/shaders/3d/Texture3DDeferred.frag", GL_FRAGMENT_SHADER);
        mat.deferred = ShaderManager::createProgram({vert, frag});

        mat.deferred.use();
        mat.deferred.setUniform("F0", F0);

        vert = ShaderManager::compileShaderFile("res/shaders/3d/Texture3DDepth.vert", GL_VERTEX_SHADER);
        frag = ShaderManager::compileShaderFile("res/shaders/3d/Texture3DDepth.frag", GL_FRAGMENT_SHADER);
        mat.depth = ShaderManager::createProgram({vert, frag});

        GLTexture tex[]{albedo, normal, roughnessMetallic};
        mat.textures.addAll(tex, 3u);

        return mat;
    }
};

#endif