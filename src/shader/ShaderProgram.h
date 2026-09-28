#ifndef SHADERPROGRAM_H_INCLUDED
#define SHADERPROGRAM_H_INCLUDED

#pragma once

#include <cstdint>
#include <type_traits>

#include <glad/glad.h>
#include <prvl.h>

template <typename>
inline constexpr bool always_false = false;

struct ShaderProgram {
    GLuint ID = 0u;
    uint32_t vertexShaderHandle = 0u;

    ShaderProgram() noexcept {
    }

    ShaderProgram(GLuint program, uint32_t vertexHandle) noexcept : ID(program), vertexShaderHandle(vertexHandle) {
    }

    inline void use() const {
        glUseProgram(ID);
    }

    template <typename T>
    void setUniform(const char* name, const T& v) const {
        GLint loc = glGetUniformLocation(ID, name);
        if (loc == -1) {
            return;
        }

        if constexpr (std::is_same_v<T, int32_t>) {
            glProgramUniform1i(ID, loc, v);
        } else if constexpr (std::is_same_v<T, ivec2>) {
            glProgramUniform2i(ID, loc, v.x, v.y);
        } else if constexpr (std::is_same_v<T, ivec3>) {
            glProgramUniform3i(ID, loc, v.x, v.y, v.z);
        } else if constexpr (std::is_same_v<T, ivec4>) {
            glProgramUniform4i(ID, loc, v.x, v.y, v.z, v.w);
        }

        else if constexpr (std::is_same_v<T, uint32_t>) {
            glProgramUniform1ui(ID, loc, v);
        } else if constexpr (std::is_same_v<T, uvec2>) {
            glProgramUniform2ui(ID, loc, v.x, v.y);
        } else if constexpr (std::is_same_v<T, uvec3>) {
            glProgramUniform3ui(ID, loc, v.x, v.y, v.z);
        } else if constexpr (std::is_same_v<T, uvec4>) {
            glProgramUniform4ui(ID, loc, v.x, v.y, v.z, v.w);
        }

        else if constexpr (std::is_same_v<T, float>) {
            glProgramUniform1f(ID, loc, v);
        } else if constexpr (std::is_same_v<T, vec2>) {
            glProgramUniform2f(ID, loc, v.x, v.y);
        } else if constexpr (std::is_same_v<T, vec3>) {
            glProgramUniform3f(ID, loc, v.x, v.y, v.z);
        } else if constexpr (std::is_same_v<T, vec4>) {
            glProgramUniform4f(ID, loc, v.x, v.y, v.z, v.w);
        }

        else if constexpr (std::is_same_v<T, double>) {
            glProgramUniform1d(ID, loc, v);
        } else if constexpr (std::is_same_v<T, dvec2>) {
            glProgramUniform2d(ID, loc, v.x, v.y);
        } else if constexpr (std::is_same_v<T, dvec3>) {
            glProgramUniform3d(ID, loc, v.x, v.y, v.z);
        } else if constexpr (std::is_same_v<T, dvec4>) {
            glProgramUniform4d(ID, loc, v.x, v.y, v.z, v.w);
        } else {
            static_assert(always_false<T>, "Unsupported uniform type");
        }
    }

    template <typename T>
    void setUniform(const char* name, const T* v, uint32_t count) const {
        GLint loc = glGetUniformLocation(ID, name);
        if (loc == -1) {
            return;
        }
        if constexpr (std::is_same_v<T, int32_t>) {
            glProgramUniform1iv(ID, loc, count, v);
        } else if constexpr (std::is_same_v<T, ivec2>) {
            glProgramUniform2iv(ID, loc, count, reinterpret_cast<const int32_t*>(v));
        } else if constexpr (std::is_same_v<T, ivec3>) {
            glProgramUniform3iv(ID, loc, count, reinterpret_cast<const int32_t*>(v));
        } else if constexpr (std::is_same_v<T, ivec4>) {
            glProgramUniform4iv(ID, loc, count, reinterpret_cast<const int32_t*>(v));
        }

        else if constexpr (std::is_same_v<T, uint32_t>) {
            glProgramUniform1uiv(ID, loc, count, v);
        } else if constexpr (std::is_same_v<T, uvec2>) {
            glProgramUniform2uiv(ID, loc, count, reinterpret_cast<const uint32_t*>(v));
        } else if constexpr (std::is_same_v<T, uvec3>) {
            glProgramUniform3uiv(ID, loc, count, reinterpret_cast<const uint32_t*>(v));
        } else if constexpr (std::is_same_v<T, uvec4>) {
            glProgramUniform4uiv(ID, loc, count, reinterpret_cast<const uint32_t*>(v));
        }

        else if constexpr (std::is_same_v<T, float>) {
            glProgramUniform1fv(ID, loc, count, v);
        } else if constexpr (std::is_same_v<T, vec2>) {
            glProgramUniform2fv(ID, loc, count, reinterpret_cast<const float*>(v));
        } else if constexpr (std::is_same_v<T, vec3>) {
            glProgramUniform3fv(ID, loc, count, reinterpret_cast<const float*>(v));
        } else if constexpr (std::is_same_v<T, vec4>) {
            glProgramUniform4fv(ID, loc, count, reinterpret_cast<const float*>(v));
        }

        else if constexpr (std::is_same_v<T, double>) {
            glProgramUniform1dv(ID, loc, count, v);
        } else if constexpr (std::is_same_v<T, dvec2>) {
            glProgramUniform2dv(ID, loc, count, reinterpret_cast<const double*>(v));
        } else if constexpr (std::is_same_v<T, dvec3>) {
            glProgramUniform3dv(ID, loc, count, reinterpret_cast<const double*>(v));
        } else if constexpr (std::is_same_v<T, dvec4>) {
            glProgramUniform4dv(ID, loc, count, reinterpret_cast<const double*>(v));
        } else {
            static_assert(always_false<T>, "Unsupported uniform type");
        }
    }

    void dispatchCompute(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ, GLbitfield barriers) const {
        glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
        glMemoryBarrier(barriers);
    }

    explicit inline operator bool() const noexcept {
        return ID != 0;
    }
};

#endif
