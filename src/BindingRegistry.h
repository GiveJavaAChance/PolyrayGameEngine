#ifndef BINDINGREGISTRY_H_INCLUDED
#define BINDINGREGISTRY_H_INCLUDED

#pragma once

#include <unordered_map>
#include "GLTexture.h"
#include "ShaderBuffer.h"

namespace BindingRegistry {
    namespace Internal {
        inline std::unordered_map<GLuint, GLuint> texBindings;
        inline std::unordered_map<GLuint, GLuint> bufBindings;

        inline int getNewBinding(std::unordered_map<GLuint, GLuint>& usedBindings, GLuint max) {
            GLuint maxSearchRange = 0u;
            for (std::pair<GLuint, GLuint> i : usedBindings) {
                if (i.second > maxSearchRange) {
                    maxSearchRange = i.second;
                }
            }
            maxSearchRange += 2u;
            if (maxSearchRange > max) {
                maxSearchRange = max;
            }
            for (GLuint i = 0; i < max; i++) {
                bool used = false;
                for (std::pair<GLuint, GLuint> kv : usedBindings) {
                    if (kv.second == i) {
                        used = true;
                        break;
                    }
                }
                if (!used) return i;
            }
            return -1;
        }
    }

    using namespace Internal;

    inline GLuint bindImageTexture(const GLTexture& texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format) {
        auto it = texBindings.find(texture.ID);
        if (it != texBindings.end()) {
            return it->second;
        }
        GLint max;
        glGetIntegerv(GL_MAX_IMAGE_UNITS, &max);
        int binding = getNewBinding(texBindings, max);
        if (binding == -1) {
            std::cerr << "No available image binding points!" << std::endl;
            return 0xFFFFFFFFu;
        }
        glBindImageTexture(binding, texture.ID, level, layered, layer, access, format);
        texBindings[texture.ID] = binding;
        return binding;
    }

    inline GLuint bindImageTexture(const GLTexture& texture, GLint level, GLboolean layered, GLint layer, GLenum access) {
        return bindImageTexture(texture, level, layered, layer, access, texture.format);
    }

    inline GLuint bindBufferBase(ShaderBuffer& buffer, GLenum target) {
        auto it = bufBindings.find(buffer.ID);
        if (it != bufBindings.end()) {
            return it->second;
        }
        GLint max;
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &max);
        int binding = getNewBinding(bufBindings, max);
        if (binding == -1) {
            std::cerr << "No available image binding points!" << std::endl;
            return 0xFFFFFFFFu;
        }
        glBindBufferBase(target, binding, buffer.ID);
        bufBindings[buffer.ID] = binding;
        return binding;
    }

    inline void unbindImageTexture(const GLTexture& texture) {
        texBindings.erase(texture.ID);
    }

    inline void unbindImageTexture(const ShaderBuffer& buffer) {
        bufBindings.erase(buffer.ID);
    }
}

#endif
