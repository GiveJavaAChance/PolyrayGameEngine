#ifndef BINDINGREGISTRY_H_INCLUDED
#define BINDINGREGISTRY_H_INCLUDED

#pragma once

#include <unordered_map>
#include <rendering/GLTexture.h>
#include <rendering/ShaderBuffer.h>
#include <structure/IDGenerator.h>

namespace BindingRegistry {
    namespace Internal {
        inline IDGenerator bufferGen;
        inline IDGenerator textureGen;
        inline std::unordered_map<GLuint, GLuint> texBindings;
        inline std::unordered_map<GLuint, GLuint> bufBindings;

        inline uint32_t getNewBinding(IDGenerator& gen, GLuint max) {
            uint32_t binding = gen.getNewID();
            if(binding > max) {
                return UINT32_MAX;
            }
            return binding;
        }
    }

    using namespace Internal;

    inline GLuint allocateTextureBinding() {
        GLint max;
        glGetIntegerv(GL_MAX_IMAGE_UNITS, &max);
        return getNewBinding(textureGen, max);
    }

    inline GLuint allocateBufferBinding() {
        GLint max;
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &max);
        return getNewBinding(bufferGen, max);
    }

    inline void freeTextureBinding(GLuint binding) {
        textureGen.free(binding);
    }

    inline void freeBufferBinding(GLuint binding) {
        bufferGen.free(binding);
    }

    inline GLuint bindImageTexture(const GLTexture& texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format) {
        auto it = texBindings.find(texture.ID);
        if (it != texBindings.end()) {
            return it->second;
        }
        GLint max;
        glGetIntegerv(GL_MAX_IMAGE_UNITS, &max);
        uint32_t binding = getNewBinding(textureGen, max);
        if (binding == UINT32_MAX) {
            std::cerr << "No available image binding points!" << std::endl;
            return UINT32_MAX;
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
        uint32_t binding = getNewBinding(bufferGen, max);
        if (binding == UINT32_MAX) {
            std::cerr << "No available image binding points!" << std::endl;
            return UINT32_MAX;
        }
        glBindBufferBase(target, binding, buffer.ID);
        bufBindings[buffer.ID] = binding;
        return binding;
    }

    inline void unbindImageTexture(const GLTexture& texture) {
        freeTextureBinding(texBindings[texture.ID]);
        texBindings.erase(texture.ID);
    }

    inline void unbindImageTexture(const ShaderBuffer& buffer) {
        freeBufferBinding(bufBindings[buffer.ID]);
        bufBindings.erase(buffer.ID);
    }
}

#endif
