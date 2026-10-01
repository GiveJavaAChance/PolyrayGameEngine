#ifndef SHADERBUFFER_H_INCLUDED
#define SHADERBUFFER_H_INCLUDED

#pragma once

#include <cstdint>

#include <glad/glad.h>

#include <prvl.h>
#include <utils/prvl_assert.h>

struct ShaderBuffer {
    GLbitfield flags;

    GLuint ID;
    size_t size;

    ShaderBuffer() : flags(0ull), ID(0u), size(0ull) {
    }

    ShaderBuffer(size_t size, GLbitfield flags = GL_DYNAMIC_STORAGE_BIT) : flags(flags), size(size) {
        glCreateBuffers(1, &ID);
        glNamedBufferStorage(ID, static_cast<GLsizeiptr>(size), nullptr, flags);
    }

    void uploadData(const void* data, GLsizeiptr size, GLintptr offset = 0ll) const {
        glNamedBufferSubData(ID, offset, size, data);
    }

    void* map(GLbitfield access, size_t offset = 0ull, size_t size = SIZE_MAX) const {
        PRVL_ASSERT(access == GL_MAP_READ_BIT || access == GL_MAP_WRITE_BIT, "Buffer mapping access must be read or write");
        PRVL_ASSERT(flags & access, "Buffer was not created with the requested mapping access");

        access |= flags & (GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

        GLsizeiptr mapSize = size == SIZE_MAX ? this->size - offset : size;
        return glMapNamedBufferRange(ID, offset, mapSize, access);
    }

    void destroy() {
        if (ID != 0u) {
            glDeleteBuffers(1, &ID);
            ID = 0u;
            size = 0ull;
        }
    }

    explicit inline operator bool() const noexcept {
        return ID != 0u;
    }

    static void copy(const ShaderBuffer& dst, const ShaderBuffer& src, size_t dstOffset = 0ull, size_t srcOffset = 0ull, size_t size = SIZE_MAX) {
        GLsizeiptr copySize = size == SIZE_MAX ? min(src.size - srcOffset, dst.size - dstOffset) : size;
        glCopyNamedBufferSubData(src.ID, dst.ID, srcOffset, dstOffset, copySize);
    }

    static size_t getUniformBufferStride(size_t size) {
        GLint alignment;
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
        return (size + alignment - 1ull) / alignment * alignment;
    }
};

#endif
