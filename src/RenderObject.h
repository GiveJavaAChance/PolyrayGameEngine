#ifndef RENDEROBJECT_H_INCLUDED
#define RENDEROBJECT_H_INCLUDED

#pragma once

#include <cstdint>

#include <glad/glad.h>
#include <rendering/ShaderBuffer.h>
#include <shader/ShaderManager.h>

struct RenderObject {
    ShaderBuffer vbo;
    ShaderBuffer instanceVbo;
    GLuint vao;
    uint32_t vertexCount = 0u;
    uint32_t instanceCount = 0u;
    GLenum mode = GL_TRIANGLES;

    RenderObject(const VertexLayoutInfo& layout) : vbo(GL_STATIC_DRAW), instanceVbo(GL_STATIC_DRAW), vao(ShaderManager::createVAO(layout, {vbo.ID, instanceVbo.ID})) {
    }

    template <typename T>
    void uploadVertices(const T* vertices, uint32_t count) {
        if (count == vertexCount) {
            vbo.uploadPartialData(vertices, vertexCount, 0);
        } else {
            vbo.uploadData<T>(vertices, count);
            vertexCount = count;
        }
    }

    template <typename T>
    void uploadInstances(const T* instances, uint32_t count) {
        if (count == instanceCount) {
            instanceVbo.uploadPartialData<T>(instances, instanceCount, 0);
        } else {
            instanceVbo.uploadData<T>(instances, count);
            instanceCount = count;
        }
    }

    void render() {
        if (vertexCount == 0u || instanceCount == 0u) {
            return;
        }
        glBindVertexArray(vao);
        glDrawArraysInstanced(mode, 0, vertexCount, instanceCount);
    }
};

#endif
