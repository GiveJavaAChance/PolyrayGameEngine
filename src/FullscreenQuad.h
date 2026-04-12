#ifndef FULLSCREENQUAD_H_INCLUDED
#define FULLSCREENQUAD_H_INCLUDED

#pragma once

#include <cstdint>

#include <base/glad/glad.h>
#include <rendering/GLTexture.h>
#include <rendering/ShaderBuffer.h>
#include <shader/ShaderManager.h>
#include <shader/ShaderProgram.h>

struct FullscreenQuad {
    GLuint vao;
    ShaderBuffer vbo;

    ShaderProgram quadProgram;
    GLTexture* texture = nullptr;

    FullscreenQuad(uint32_t fragmentShader) : vbo(GL_STATIC_DRAW) {
        uint32_t vertexShader = ShaderManager::compileShaderSource("#version 330 core\nlayout(location = 0) in vec2 pos;\nout vec2 uv;\nvoid main() {\nuv = pos * 0.5 + 0.5;\ngl_Position = vec4(pos, 0.0, 1.0);\n}", GL_VERTEX_SHADER, false);
        quadProgram = ShaderManager::createProgram({vertexShader, fragmentShader});
        vao = ShaderManager::createVAO(quadProgram, {vbo.ID});
        float quadVertices[]{
            -1.0f, 1.0f,
            -1.0f, -1.0f,
            1.0f, 1.0f,
            1.0f, -1.0f
        };
        vbo.uploadData(quadVertices, 8);
    }

    FullscreenQuad() : FullscreenQuad(ShaderManager::compileShaderSource("#version 330 core\nin vec2 uv;\nout vec4 color;\nuniform sampler2D screenTexture;\nvoid main() {\ncolor = texture(screenTexture, uv);\n}", GL_FRAGMENT_SHADER, false)) {
    }

    void setTexture(GLTexture* texture) {
        this->texture = texture;
    }

    void render() {
        if(!texture) {
            return;
        }
        quadProgram.use();
        glBindTextureUnit(0, texture->ID);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
};

#endif
