#ifndef VERTEXBUFFERTEMPLATE_H_INCLUDED
#define VERTEXBUFFERTEMPLATE_H_INCLUDED

#pragma once

#include <initializer_list>
#include "Allocator.h"

struct VertexAttribute {
    GLint size;
    GLenum type;
    GLboolean normalized;

    constexpr VertexAttribute() noexcept : size(1), type(GL_FLOAT), normalized(GL_FALSE) {
    }

    constexpr VertexAttribute(GLint size, GLenum type, GLboolean normalized) noexcept : size(size), type(type), normalized(normalized) {
    }

    constexpr uint32_t byteSize() const noexcept {
        return size * (
            type == GL_FLOAT ? 4 :
            type == GL_INT   ? 4 :
            type == GL_SHORT ? 2 :
            1
        );
    }

    void bind(const GLuint idx, const int off, const GLint totalSize) const {
        glVertexAttribPointer(idx, size, type, normalized, totalSize, (const void*) (uintptr_t) off);
        glEnableVertexAttribArray(idx);
    }

    inline static constexpr VertexAttribute FLOAT() { return {1, GL_FLOAT, GL_FALSE}; }
    inline static constexpr VertexAttribute VEC2()  { return {2, GL_FLOAT, GL_FALSE}; }
    inline static constexpr VertexAttribute VEC3()  { return {3, GL_FLOAT, GL_FALSE}; }
    inline static constexpr VertexAttribute VEC4()  { return {4, GL_FLOAT, GL_FALSE}; }
    inline static constexpr VertexAttribute INT()   { return {1, GL_INT,   GL_FALSE}; }
    inline static constexpr VertexAttribute IVEC2() { return {2, GL_INT,   GL_FALSE}; }
    inline static constexpr VertexAttribute IVEC3() { return {3, GL_INT,   GL_FALSE}; }
    inline static constexpr VertexAttribute IVEC4() { return {4, GL_INT,   GL_FALSE}; }
};


struct VertexBufferTemplate {
    bool instanced;
    uint32_t attributeCount;
    VertexAttribute* attributes;
    GLint totalSize;

    VertexBufferTemplate() = default;

    VertexBufferTemplate(const bool instanced, const std::initializer_list<VertexAttribute> attribs) : instanced(instanced), attributeCount(attribs.size()) {
        attributes = alloc<VertexAttribute>(attributeCount);
        std::copy(attribs.begin(), attribs.end(), attributes);
        totalSize = 0;
        for (uint32_t i = 0u; i < attributeCount; i++) {
            totalSize += attributes[i].byteSize();
        }
    }

    void build(GLuint idx, GLuint vbo) const {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        int off = 0;
        for (uint32_t i = 0u; i < attributeCount; i++) {
            attributes[i].bind(idx, off, totalSize);
            if (instanced) {
                glVertexAttribDivisor(idx, 1);
            }
            idx++;
            off += attributes[i].byteSize();
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint build(GLuint idx) const {
        GLuint vbo;
        glGenBuffers(1, &vbo);
        build(idx, vbo);
        return vbo;
    }

    inline GLint getSize() const {
        return totalSize;
    }

    inline uint32_t getAttributeCount() const {
        return attributeCount;
    }
};

#endif
