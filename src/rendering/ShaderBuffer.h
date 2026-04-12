#ifndef SHADERBUFFER_H_INCLUDED
#define SHADERBUFFER_H_INCLUDED

#pragma once

#include <cstdint>
#include <base/glad/glad.h>

struct ShaderBuffer {
private:
    GLenum usage;

public:
    GLuint ID;

    ShaderBuffer(GLenum usage) : usage(usage) {
        glCreateBuffers(1, &ID);
    }

    ShaderBuffer(GLenum usage, GLuint id) : usage(usage), ID(id) {
    }

    void setUsage(GLenum newUsage) {
        usage = newUsage;
    }

    void setSize(GLsizeiptr size) const {
        glNamedBufferData(ID, size, nullptr, usage);
    }

    void uploadData(const void* data, GLsizeiptr bytes) const {
        glNamedBufferData(ID, bytes, data, usage);
    }

    template<typename T>
    void uploadData(const T* data, GLsizeiptr count) const {
        glNamedBufferData(ID, count * sizeof(T), data, usage);
    }

    void uploadPartialData(const void* data, GLsizeiptr bytes, GLintptr byteOffset) const {
        glNamedBufferSubData(ID, byteOffset, bytes, data);
    }

    template<typename T>
    void uploadPartialData(const T* data, GLsizeiptr count, GLintptr offset) const {
        glNamedBufferSubData(ID, offset * sizeof(T), count * sizeof(T), data);
    }

    void deleteBuffer() {
        glDeleteBuffers(1, &ID);
    }

    explicit inline operator bool() const noexcept {
        return ID != 0;
    }
};

#endif
