#ifndef SHADERMANAGER_H_INCLUDED
#define SHADERMANAGER_H_INCLUDED

#pragma once

#include <algorithm>
#include <charconv>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <ResourceManager.h>
#include <shader/ShaderPreprocessor.h>
#include <shader/ShaderProgram.h>
#include <structure/UnorderedRegistry.h>

struct VertexAttribInfo {
    std::string name;
    GLuint location;
    GLenum baseType;
    GLenum type;
    GLint columns;
    GLint components;
    GLsizei byteSize;
    bool instanced;
    uint32_t vboIdx;
    bool optional;
};

struct VertexLayoutInfo {
    std::vector<VertexAttribInfo> attributes;
    bool hasInstanceFrom;
    uint32_t instanceFrom;
    uint32_t vboCount;
    std::vector<uint32_t> vboStrides;
};

namespace ShaderManager {
    namespace Internal {
        struct CompiledShader {
            GLuint ID;
            GLenum type;

            bool hasInstanceFrom;
            uint32_t instanceFrom;
            std::unordered_map<std::string, uint32_t> attributeMapping;
            uint32_t maxMapping;

            std::unordered_set<std::string> optionalAttributes;
        };

        inline UnorderedRegistry<CompiledShader> shaders;

        inline std::unordered_map<std::string, uint32_t> shaderCache;

        inline std::unordered_set<GLuint> programs;

        inline ShaderPreprocessor proc;
    }

    using namespace Internal;

    inline void setValue(const char* name, const char* value) {
        proc.setValue(name, value);
    }

    template <typename T>
    inline void setValue(const char* name, const T& value) {
        proc.setValue<T>(name, value);
    }

    inline void removeValue(const char* name) {
        proc.removeValue(name);
    }

    inline uint32_t compileShaderSource(const char* source, GLenum type, bool preprocess = true) {
        GLuint shader = glCreateShader(type);
        int32_t instanceFrom = -1;
        std::unordered_map<std::string, uint32_t> attributeMap;
        std::unordered_set<std::string> optionalAttributes;
        uint32_t maxMapping = 1u;
        std::string src(source);
        ShaderPreprocessor::clean(src);
        if (type == GL_VERTEX_SHADER) {
            std::string found;
            if (ShaderPreprocessor::findDirective(src, "instancefrom", found)) {
                if (std::from_chars(found.data(), found.data() + found.size(), instanceFrom).ec != std::errc()) {
                    instanceFrom = -1;
                }
                maxMapping = 2u;
            }
            if (ShaderPreprocessor::findDirective(src, "attributemap", found)) {
                std::vector<std::pair<std::string, std::string>> maps;
                ShaderPreprocessor::extractDirectiveList(found, maps);
                for (std::pair<std::string, std::string>& m : maps) {
                    uint32_t idx;
                    if (std::from_chars(m.second.data(), m.second.data() + m.second.size(), idx).ec == std::errc()) {
                        maxMapping = max(maxMapping, idx + 1u);
                        attributeMap[m.first] = idx;
                    }
                }
            }
            while (ShaderPreprocessor::findDirective(src, "optional", found)) {
                optionalAttributes.insert(found);
            }
        }
        if (preprocess) {
            proc.process(src);
        }
        const char* processed = src.c_str();
        glShaderSource(shader, 1, &processed, nullptr);
        glCompileShader(shader);
        GLint status = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE) {
            GLint logLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
            if (logLen > 1) {
                std::string log(logLen, '\0');
                glGetShaderInfoLog(shader, logLen, nullptr, log.data());
                std::cerr << log << std::endl;
            }
            glDeleteShader(shader);
            return 0u;
        } else {
            return shaders.emplace(shader, type, instanceFrom >= 0, instanceFrom < 0 ? 0u : static_cast<uint32_t>(instanceFrom), attributeMap, maxMapping, optionalAttributes) + 1u;
        }
    }

    inline uint32_t compileShaderFile(const char* name, GLenum type, bool preprocess = true) {
        if (shaderCache.count(name)) {
            return shaderCache[name];
        }
        std::string src = ResourceManager::getResourceAsString(name);
        return compileShaderSource(src.c_str(), type, preprocess);
    }

    inline void deleteShader(uint32_t shader) {
        const CompiledShader& s = shaders[shader - 1u];
        glDeleteShader(s.ID);
        shaders.remove(shader - 1u);
    }

    inline ShaderProgram createProgram(std::initializer_list<uint32_t> shaders) {
        bool incomplete = false;
        GLuint program = glCreateProgram();
        for (const uint32_t shader : shaders) {
            glAttachShader(program, Internal::shaders[shader - 1u].ID);
        }
        glLinkProgram(program);
        GLint status = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status != GL_TRUE) {
            GLint logLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
            if (logLen > 1) {
                std::string log(logLen, '\0');
                glGetProgramInfoLog(program, logLen, nullptr, log.data());
                std::cerr << log << std::endl;
            }
            incomplete = true;
        }
        glValidateProgram(program);
        status = GL_FALSE;
        glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
        if (status != GL_TRUE) {
            GLint logLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
            if (logLen > 1) {
                std::string log(logLen, '\0');
                glGetProgramInfoLog(program, logLen, nullptr, log.data());
                std::cerr << log << std::endl;
            }
            incomplete = true;
        }
        if (incomplete) {
            glDeleteProgram(program);
            return ShaderProgram();
        }
        uint32_t vertexShaderHandle = 0u;
        for (uint32_t shader : shaders) {
            const CompiledShader& s = Internal::shaders[shader - 1u];
            if (s.type == GL_VERTEX_SHADER) {
                vertexShaderHandle = shader;
                break;
            }
        }
        programs.insert(program);
        return ShaderProgram(program, vertexShaderHandle);
    }

    inline VertexLayoutInfo getVertexLayout(const ShaderProgram& program) {
        VertexLayoutInfo layout;

        const CompiledShader& vertexShader = shaders[program.vertexShaderHandle - 1u];
        layout.hasInstanceFrom = vertexShader.hasInstanceFrom;
        layout.instanceFrom = vertexShader.instanceFrom;

        GLint attribCount = 0;
        glGetProgramiv(program.ID, GL_ACTIVE_ATTRIBUTES, &attribCount);

        constexpr GLsizei bufSize = 256;
        char nameBuf[bufSize];

        for (GLint i = 0; i < attribCount; i++) {
            GLsizei length;
            GLint size;
            GLenum type;
            glGetActiveAttrib(program.ID, i, bufSize, &length, &size, &type, nameBuf);

            if (strncmp(nameBuf, "gl_", 3) == 0) {
                continue;
            }

            GLuint location = glGetAttribLocation(program.ID, nameBuf);

            VertexAttribInfo attr;
            attr.name = nameBuf;
            attr.location = location;
            attr.type = type;
            switch (type) {
                case GL_FLOAT:
                case GL_INT:
                case GL_UNSIGNED_INT:
                    attr.columns = 1;
                    attr.components = 1;
                    break;
                case GL_FLOAT_VEC2:
                case GL_INT_VEC2:
                case GL_UNSIGNED_INT_VEC2:
                    attr.columns = 1;
                    attr.components = 2;
                    break;
                case GL_FLOAT_VEC3:
                case GL_INT_VEC3:
                case GL_UNSIGNED_INT_VEC3:
                    attr.columns = 1;
                    attr.components = 3;
                    break;
                case GL_FLOAT_VEC4:
                case GL_INT_VEC4:
                case GL_UNSIGNED_INT_VEC4:
                    attr.columns = 1;
                    attr.components = 4;
                    break;

                case GL_FLOAT_MAT2:
                    attr.columns = 2;
                    attr.components = 2;
                    break;
                case GL_FLOAT_MAT3:
                    attr.columns = 3;
                    attr.components = 3;
                    break;
                case GL_FLOAT_MAT4:
                    attr.columns = 4;
                    attr.components = 4;
                    break;

                default:
                    std::cerr << "Unsupported vertex attrib type: " << type << " at location " << location << "\n";
                    continue;
            }
            switch (type) {
                case GL_FLOAT:
                case GL_FLOAT_VEC2:
                case GL_FLOAT_VEC3:
                case GL_FLOAT_VEC4:
                case GL_FLOAT_MAT2:
                case GL_FLOAT_MAT3:
                case GL_FLOAT_MAT4:
                    attr.baseType = GL_FLOAT;
                    break;

                case GL_INT:
                case GL_INT_VEC2:
                case GL_INT_VEC3:
                case GL_INT_VEC4:
                    attr.baseType = GL_INT;
                    break;

                case GL_UNSIGNED_INT:
                case GL_UNSIGNED_INT_VEC2:
                case GL_UNSIGNED_INT_VEC3:
                case GL_UNSIGNED_INT_VEC4:
                    attr.baseType = GL_UNSIGNED_INT;
                    break;
            }
            attr.byteSize = attr.components * 4;
            attr.instanced = (layout.hasInstanceFrom && location >= layout.instanceFrom);

            auto it = vertexShader.attributeMapping.find(nameBuf);
            if (it != vertexShader.attributeMapping.end()) {
                attr.vboIdx = it->second;
            } else {
                attr.vboIdx = attr.instanced ? 1u : 0u;
            }

            attr.optional = vertexShader.optionalAttributes.contains(nameBuf);

            layout.attributes.push_back(attr);
        }
        std::vector<VertexAttribInfo>& attrs = layout.attributes;
        std::sort(attrs.begin(), attrs.end(), [](const VertexAttribInfo& a, const VertexAttribInfo& b) {
            return a.location < b.location;
        });
        layout.vboCount = 0u;
        for (const VertexAttribInfo& attr : layout.attributes) {
            layout.vboCount = max(layout.vboCount, attr.vboIdx + 1u);
        }
        layout.vboStrides.resize(layout.vboCount);
        for (const VertexAttribInfo& attr : layout.attributes) {
            layout.vboStrides[attr.vboIdx] += attr.columns * attr.byteSize;
        }
        return layout;
    }

    inline void deleteProgram(const ShaderProgram& program) {
        programs.erase(program.ID);
        glDeleteProgram(program.ID);
    }

    inline GLuint createVAO(const VertexLayoutInfo& layout) {
        GLuint vao;
        glCreateVertexArrays(1, &vao);
        std::vector<uint32_t> vboOffsets(layout.vboCount, 0);
        for (const VertexAttribInfo& attr : layout.attributes) {
            GLuint bindingIndex = attr.vboIdx;
            uint32_t offset = vboOffsets[attr.vboIdx];
            for (GLint col = 0; col < attr.columns; col++) {
                GLuint attribLocation = attr.location + col;
                if (attr.baseType == GL_FLOAT) {
                    glVertexArrayAttribFormat(vao, attribLocation, attr.components, attr.baseType, GL_FALSE, offset + col * attr.byteSize);
                } else {
                    glVertexArrayAttribIFormat(vao, attribLocation, attr.components, attr.baseType, offset + col * attr.byteSize);
                }
                glVertexArrayAttribBinding(vao, attribLocation, bindingIndex);
                glEnableVertexArrayAttrib(vao, attribLocation);
                if (attr.instanced) {
                    glVertexArrayBindingDivisor(vao, bindingIndex, 1);
                }
            }
            vboOffsets[attr.vboIdx] += attr.columns * attr.byteSize;
        }
        return vao;
    }

    inline GLuint setVAOBuffers(GLuint vao, const VertexLayoutInfo& layout, const std::vector<GLuint>& vbos) {
        for (const VertexAttribInfo& attr : layout.attributes) {
            GLuint bindingIndex = attr.vboIdx;
            glVertexArrayVertexBuffer(vao, bindingIndex, vbos[attr.vboIdx], 0, layout.vboStrides[attr.vboIdx]);
        }
        return vao;
    }

}

#endif
