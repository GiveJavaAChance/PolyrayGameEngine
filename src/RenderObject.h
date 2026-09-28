#ifndef RENDEROBJECT_H_INCLUDED
#define RENDEROBJECT_H_INCLUDED

#pragma once

#include <cstdint>

#include <glad/glad.h>
#include <rendering/ShaderBuffer.h>
#include <shader/ShaderManager.h>

#include <Mesh.h>

struct RenderObject {
    ShaderBuffer vbo;
    uint32_t vertexCount = 0u;
    ShaderBuffer ebo;
    uint32_t indexCount = 0u;
    ShaderBuffer instanceVbo;
    uint32_t instanceCount = 0u;
    GLuint vao;
    
    GLenum mode = GL_TRIANGLES;

    RenderObject(const VertexLayoutInfo& layout) : vbo(GL_STATIC_DRAW), ebo(GL_STATIC_DRAW), instanceVbo(GL_STATIC_DRAW), vao(ShaderManager::createVAO(layout, {vbo.ID, instanceVbo.ID})) {
        glVertexArrayElementBuffer(vao, ebo.ID);
    }

    template <typename V>
    void uploadMesh(const Mesh<V>& mesh) {
        vbo.uploadData(mesh.vertices.data(), mesh.vertices.size());
        ebo.uploadData(mesh.indices.data(), mesh.indices.size());
        vertexCount = mesh.vertices.size();
        indexCount = mesh.indices.size();
    }

    template <typename T>
    void uploadInstances(const T* instances, uint32_t count) {
        instanceVbo.uploadData<T>(instances, count);
        instanceCount = count;
    }

    void render() {
        if (vertexCount == 0u || indexCount == 0u || instanceCount == 0u) {
            return;
        }
        glBindVertexArray(vao);
        glDrawElementsInstanced(mode, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
    }
};

#endif
