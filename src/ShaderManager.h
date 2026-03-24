#ifndef SHADERMANAGER_H_INCLUDED
#define SHADERMANAGER_H_INCLUDED

#pragma once

#include <unordered_set>
#include <unordered_map>
#include <charconv>
#include <string>
#include <algorithm>
#include <vector>
#include "ResourceManager.h"
#include "ShaderPreprocessor.h"
#include "ShaderProgram.h"
#include "UnorderedRegistry.h"

namespace ShaderManager {
    namespace Internal {
        struct VertexAttribInfo {
            GLuint location;
            GLenum type;
            GLint columns;
            GLint components;
            GLsizei byteSize;
            bool instanced;
            uint32_t vboIdx;
        };

        struct VertexLayoutInfo {
            std::vector<VertexAttribInfo> attributes;
            bool hasInstanceFrom;
            uint32_t instanceFrom;
        };

        struct CompiledShader {
            GLuint ID;
            GLenum type;

            bool hasInstanceFrom;
            uint32_t instanceFrom;
            std::unordered_map<std::string, uint32_t> attributeMapping;
            uint32_t maxMapping;
        };

        inline UnorderedRegistry<CompiledShader> shaders;

        inline std::unordered_set<GLuint> programs;

        inline UnorderedRegistry<VertexLayoutInfo> vertexLayoutInfos;

        inline ShaderPreprocessor proc;

        void reflectVertexLayout(GLuint program, uint32_t& handle, const CompiledShader& vertexShader) {
            VertexLayoutInfo layout;
            layout.hasInstanceFrom = vertexShader.hasInstanceFrom;
            layout.instanceFrom = vertexShader.instanceFrom;

            GLint attribCount = 0;
            glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &attribCount);

            constexpr GLsizei bufSize = 256;
            char nameBuf[bufSize];

            for (GLint i = 0; i < attribCount; i++) {
                GLsizei length;
                GLint size;
                GLenum type;
                glGetActiveAttrib(program, i, bufSize, &length, &size, &type, nameBuf);

                if (strncmp(nameBuf, "gl_", 3) == 0) {
                    continue;
                }

                GLuint location = glGetAttribLocation(program, nameBuf);

                VertexAttribInfo attr;
                attr.location = location;
                attr.type = type;
                switch(type) {
                    case GL_FLOAT:      case GL_INT:      attr.columns = 1; attr.components = 1; break;
                    case GL_FLOAT_VEC2: case GL_INT_VEC2: attr.columns = 1; attr.components = 2; break;
                    case GL_FLOAT_VEC3: case GL_INT_VEC3: attr.columns = 1; attr.components = 3; break;
                    case GL_FLOAT_VEC4: case GL_INT_VEC4: attr.columns = 1; attr.components = 4; break;

                    case GL_FLOAT_MAT2: attr.columns = 2; attr.components = 2; break;
                    case GL_FLOAT_MAT3: attr.columns = 3; attr.components = 3; break;
                    case GL_FLOAT_MAT4: attr.columns = 4; attr.components = 4; break;

                    default:
                        std::cerr << "Unsupported vertex attrib type: " << type << " at location " << location << "\n";
                        continue;
                }
                attr.byteSize = attr.components * 4;
                attr.instanced = (layout.hasInstanceFrom && location >= layout.instanceFrom);

                auto it = vertexShader.attributeMapping.find(nameBuf);
                if (it != vertexShader.attributeMapping.end()) {
                    attr.vboIdx = it->second;
                } else {
                    attr.vboIdx = attr.instanced ? 1u : 0u;
                }
                layout.attributes.push_back(attr);
            }
            std::vector<VertexAttribInfo>& attrs = layout.attributes;
            std::sort(attrs.begin(), attrs.end(), [](const VertexAttribInfo& a, const VertexAttribInfo& b) {
                return a.location < b.location;
            });

            handle = vertexLayoutInfos.add(layout);
        }
    }

    using namespace Internal;

    inline void setValue(const char* name, const char* value) {
        proc.setValue(name, value);
    }

    template<typename T>
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
        uint32_t maxMapping = 1u;
        std::string src(source);
        ShaderPreprocessor::clean(src);
        if(type == GL_VERTEX_SHADER) {
            std::string found;
            if(ShaderPreprocessor::findDirective(src, "instancefrom", found)) {
                if (std::from_chars(found.data(), found.data() + found.size(), instanceFrom).ec != std::errc()) {
                    instanceFrom = -1;
                }
                maxMapping = 2u;
            }
            if(ShaderPreprocessor::findDirective(src, "attributemap", found)) {
                std::vector<std::pair<std::string, std::string>> maps;
                ShaderPreprocessor::extractDirectiveList(found, maps);
                for(std::pair<std::string, std::string>& m : maps) {
                    uint32_t idx;
                    if (std::from_chars(m.second.data(), m.second.data() + m.second.size(), idx).ec == std::errc()) {
                        maxMapping = std::max(maxMapping, idx + 1u);
                        attributeMap[m.first] = idx;
                    }
                }
            }
        }
        if(preprocess) {
            proc.process(src);
            std::cout << src << std::endl;
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
            return shaders.emplace(shader, type, instanceFrom >= 0, instanceFrom < 0 ? 0u : static_cast<uint32_t>(instanceFrom), attributeMap, maxMapping) + 1u;
        }
    }

    inline uint32_t compileShaderFile(const char* name, GLenum type, bool preprocess = true) {
        std::string src = ResourceManager::getResourceAsString(name);
        return compileShaderSource(src.c_str(), type, preprocess);
    }

    inline void deleteShader(const uint32_t shader) {
        const CompiledShader& s = shaders[shader - 1u];
        glDeleteShader(s.ID);
        shaders.remove(shader - 1u);
    }

    inline ShaderProgram createProgram(std::initializer_list<uint32_t> shaders) {
        bool incomplete = false;
        GLuint program = glCreateProgram();
        uint32_t handle = 0xFFFFFFFFu;
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
        if(incomplete) {
            glDeleteProgram(program);
            return ShaderProgram();
        }

        for (uint32_t shader : shaders) {
            const CompiledShader& s = Internal::shaders[shader - 1u];
            if (s.type == GL_VERTEX_SHADER) {
                reflectVertexLayout(program, handle, s);
                break;
            }
        }
        programs.insert(program);
        return ShaderProgram(program, handle);
    }

    inline void deleteProgram(const ShaderProgram& shader) {
        programs.erase(shader.ID);
        glDeleteProgram(shader.ID);
        vertexLayoutInfos.remove(shader.shaderHandle);
    }

    GLuint createVAO(const ShaderProgram& shader, const std::vector<GLuint>& vbos) {
        if(shader.shaderHandle == 0xFFFFFFFFu) {
            std::cerr << "No vertex layout info for shader " << shader.ID << "\n";
            return 0;
        }
        const VertexLayoutInfo& layout = vertexLayoutInfos[shader.shaderHandle];

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        std::vector<uint32_t> vboStrides(vbos.size(), 0);
        for (const VertexAttribInfo& attr : layout.attributes) {
            vboStrides[attr.vboIdx] += attr.columns * attr.byteSize;
        }

        std::vector<uint32_t> vboOffsets(vbos.size(), 0);

        for (const VertexAttribInfo& attr : layout.attributes) {
            glBindBuffer(GL_ARRAY_BUFFER, vbos[attr.vboIdx]);
            uint32_t offset = vboOffsets[attr.vboIdx];
            for (GLint col = 0; col < attr.columns; col++) {
                glVertexAttribPointer(
                    attr.location + col,
                    attr.components,
                    GL_FLOAT,
                    GL_FALSE,
                    vboStrides[attr.vboIdx],
                    (void*) (uintptr_t) (offset + col * attr.byteSize)
                );
                glEnableVertexAttribArray(attr.location + col);

                if (attr.instanced) {
                    glVertexAttribDivisor(attr.location + col, 1);
                }
            }
            vboOffsets[attr.vboIdx] += attr.columns * attr.byteSize;
        }

        glBindVertexArray(0);
        return vao;
    }

}

#endif
