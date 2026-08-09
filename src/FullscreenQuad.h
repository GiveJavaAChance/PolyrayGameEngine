#ifndef FULLSCREENQUAD_H_INCLUDED
#define FULLSCREENQUAD_H_INCLUDED

#pragma once

#include <cstdint>

#include <glad/glad.h>
#include <rendering/GLTexture.h>
#include <rendering/ShaderBuffer.h>
#include <shader/ShaderManager.h>
#include <shader/ShaderProgram.h>

struct FullscreenQuad {
    ShaderProgram quadProgram;

    ShaderBuffer vbo;
    GLuint vao;

    GLTexture* texture = nullptr;

    FullscreenQuad(const ShaderProgram& shader) : quadProgram(shader), vbo(GL_STATIC_DRAW), vao(ShaderManager::createVAO(shader, {vbo.ID})) {
        float quadVertices[]{
            -1.0f, 1.0f,
            -1.0f, -1.0f,
            1.0f, 1.0f,
            1.0f, -1.0f};
        vbo.uploadData(quadVertices, 8);
    }

    FullscreenQuad(uint32_t fragmentShader) : FullscreenQuad(ShaderManager::createProgram({ShaderManager::compileShaderSource("#version 330 core\nlayout(location = 0) in vec2 pos;\nout vec2 uv;\nvoid main() {\nuv = pos * 0.5 + 0.5;\ngl_Position = vec4(pos, 0.0, 1.0);\n}", GL_VERTEX_SHADER, false), fragmentShader})) {
    }

    FullscreenQuad() : FullscreenQuad(ShaderManager::compileShaderSource("#version 330 core\nin vec2 uv;\nout vec4 color;\nuniform sampler2D screenTexture;\nvoid main() {\ncolor = texture(screenTexture, uv);\n}", GL_FRAGMENT_SHADER, false)) {
    }

    void setTexture(GLTexture* texture) {
        this->texture = texture;
    }

    void render() {
        quadProgram.use();
        if (texture) {
            glBindTextureUnit(0, texture->ID);
        }
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
};

#endif
