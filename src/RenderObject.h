#ifndef RENDEROBJECT_H_INCLUDED
#define RENDEROBJECT_H_INCLUDED

#pragma once

#include <cstdint>

#include <base/glad/glad.h>
#include <rendering/ShaderBuffer.h>
#include <rendering/GLTexture.h>
#include <shader/ShaderManager.h>
#include <shader/ShaderProgram.h>

struct RenderObject {
    ShaderProgram shader;
    ShaderBuffer vbo;
    ShaderBuffer instanceVbo;
    GLuint vao;
    uint32_t vertexCount = 0u;
    uint32_t instanceCount = 0u;
    GLenum mode = GL_TRIANGLES;

    GLTexture* texture;

    RenderObject(ShaderProgram shader, GLTexture* tex = nullptr) : shader(shader), vbo(GL_STATIC_DRAW), instanceVbo(GL_STATIC_DRAW), vao(ShaderManager::createVAO(shader, {vbo.ID, instanceVbo.ID})), texture(tex) {
    }

    template<typename T>
    void uploadVertices(const T* vertices, const uint32_t count) {
        if(count == vertexCount) {
            vbo.uploadPartialData(vertices, vertexCount, 0);
        } else {
            vbo.uploadData<T>(vertices, count);
            vertexCount = count;
        }
    }

    template<typename T>
    void uploadInstances(const T* instances, const uint32_t count) {
        if(count == instanceCount) {
            instanceVbo.uploadPartialData<T>(instances, instanceCount, 0);
        } else {
            instanceVbo.uploadData<T>(instances, count);
            instanceCount = count;
        }
    }
};

#endif
