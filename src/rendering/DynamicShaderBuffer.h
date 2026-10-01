#ifndef DYNAMICSHADERBUFFER_H_INCLUDED
#define DYNAMICSHADERBUFFER_H_INCLUDED

#include "glad/glad.h"
#include <cstring>
#pragma once

#include <rendering/ShaderBuffer.h>

struct DynamicShaderBuffer {
    ShaderBuffer buffer;
    size_t pos;

    uint8_t* bufferMapping;

    bool resized = false;

    DynamicShaderBuffer(size_t initialCapacity = 1ull << 20u, GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT)
        : buffer(initialCapacity, flags), pos(0ull), bufferMapping(nullptr) {
        if (flags & (GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT)) {
            bufferMapping = reinterpret_cast<uint8_t*>(buffer.map(GL_MAP_WRITE_BIT));
        }
    }

    void ensureCapacity(size_t capacity, bool move = true) {
        if (capacity <= buffer.size) {
            return;
        }
        if (move) {
            ShaderBuffer tmp((capacity * 3ull) >> 1u, buffer.flags);
            ShaderBuffer::copy(tmp, buffer);
            buffer.destroy();
            buffer = tmp;
        } else {
            buffer.destroy();
            buffer = ShaderBuffer((capacity * 3ull) >> 1u, buffer.flags);
        }
        if (buffer.flags & (GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT)) {
            bufferMapping = reinterpret_cast<uint8_t*>(buffer.map(GL_MAP_WRITE_BIT));
        }
        resized = true;
    }

    void add(const void* data, size_t size, bool move = true) {
        size_t newCap = pos + size;
        ensureCapacity(newCap, move);
        if (bufferMapping) {
            std::memcpy(bufferMapping + pos, data, size);
        } else {
            buffer.uploadData(data, size, pos);
        }
        pos = newCap;
    }

    void reserve(size_t size, bool move = true) {
        size_t newCap = pos + size;
        ensureCapacity(newCap, move);
        pos = newCap;
    }

    void resize(size_t size, bool move = true) {
        pos = size;
        ensureCapacity(size, move);
    }
};

#endif