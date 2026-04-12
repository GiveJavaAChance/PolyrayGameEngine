#ifndef SHADERPROGRAM_H_INCLUDED
#define SHADERPROGRAM_H_INCLUDED

#pragma once

#include <cstdint>
#include <base/glad/glad.h>

template<typename>
inline constexpr bool always_false = false;

struct ShaderProgram {
    GLuint ID = 0u;
    uint32_t shaderHandle = 0xFFFFFFFFu;

    ShaderProgram() noexcept {
    }

    ShaderProgram(const GLuint program, const uint32_t handle) noexcept : ID(program), shaderHandle(handle) {
    }

    inline void use() const {
        glUseProgram(ID);
    }

    template<typename T>
    void setUniform(const char* name, const T& v0) const {
        GLint loc = glGetUniformLocation(ID, name);
        if (loc == -1) {
            return;
        }
        if constexpr (std::is_same_v<T, int>) {
            glProgramUniform1i(ID, loc, v0);
        } else if constexpr (std::is_same_v<T, unsigned int>) {
            glProgramUniform1ui(ID, loc, v0);
        } else if constexpr (std::is_same_v<T, float>) {
            glProgramUniform1f(ID, loc, v0);
        } else if constexpr (std::is_same_v<T, double>) {
            glProgramUniform1d(ID, loc, v0);
        } else {
            static_assert(always_false<T>, "Unsupported uniform type");
        }
    }

    template<typename T>
    void setUniform(const char* name, const T& v0, const T& v1) const {
        GLint loc = glGetUniformLocation(ID, name);
        if (loc == -1) {
            return;
        }
        if constexpr (std::is_same_v<T, int>) {
            glProgramUniform2i(ID, loc, v0, v1);
        } else if constexpr (std::is_same_v<T, unsigned int>) {
            glProgramUniform2ui(ID, loc, v0, v1);
        } else if constexpr (std::is_same_v<T, float>) {
            glProgramUniform2f(ID, loc, v0, v1);
        } else if constexpr (std::is_same_v<T, double>) {
            glProgramUniform2d(ID, loc, v0, v1);
        } else {
            static_assert(always_false<T>, "Unsupported uniform type");
        }
    }

    template<typename T>
    void setUniform(const char* name, const T& v0, const T& v1, const T& v2) const {
        GLint loc = glGetUniformLocation(ID, name);
        if (loc == -1) {
            return;
        }
        if constexpr (std::is_same_v<T, int>) {
            glProgramUniform3i(ID, loc, v0, v1, v2);
        } else if constexpr (std::is_same_v<T, unsigned int>) {
            glProgramUniform3ui(ID, loc, v0, v1, v2);
        } else if constexpr (std::is_same_v<T, float>) {
            glProgramUniform3f(ID, loc, v0, v1, v2);
        } else if constexpr (std::is_same_v<T, double>) {
            glProgramUniform3d(ID, loc, v0, v1, v2);
        } else {
            static_assert(always_false<T>, "Unsupported uniform type");
        }
    }

    template<typename T>
    void setUniform(const char* name, const T& v0, const T& v1, const T& v2, const T& v3) const {
        GLint loc = glGetUniformLocation(ID, name);
        if (loc == -1) {
            return;
        }
        if constexpr (std::is_same_v<T, int>) {
            glProgramUniform4i(ID, loc, v0, v1, v2, v3);
        } else if constexpr (std::is_same_v<T, unsigned int>) {
            glProgramUniform4ui(ID, loc, v0, v1, v2, v3);
        } else if constexpr (std::is_same_v<T, float>) {
            glProgramUniform4f(ID, loc, v0, v1, v2, v3);
        } else if constexpr (std::is_same_v<T, double>) {
            glProgramUniform4d(ID, loc, v0, v1, v2, v3);
        } else {
            static_assert(always_false<T>, "Unsupported uniform type");
        }
    }

    template<uint32_t dim, typename T>
    void setUniform(const char* name, const T* v, const uint32_t count) const {
        GLint loc = glGetUniformLocation(ID, name);
        if (loc == -1) {
            return;
        }
        if constexpr (std::is_same_v<T, int>) {
            if constexpr (dim == 1u) {
                glProgramUniform1iv(ID, loc, count, v);
            } else if constexpr (dim == 2u) {
                glProgramUniform2iv(ID, loc, count, v);
            } else if constexpr (dim == 3u) {
                glProgramUniform3iv(ID, loc, count, v);
            } else if constexpr (dim == 4u) {
                glProgramUniform4iv(ID, loc, count, v);
            } else {
                static_assert(always_false<T>, "Unsupported vector dimension");
            }
        } else if constexpr (std::is_same_v<T, unsigned int>) {
            if constexpr (dim == 1u) {
                glProgramUniform1uiv(ID, loc, count, v);
            } else if constexpr (dim == 2u) {
                glProgramUniform2uiv(ID, loc, count, v);
            } else if constexpr (dim == 3u) {
                glProgramUniform3uiv(ID, loc, count, v);
            } else if constexpr (dim == 4u) {
                glProgramUniform4uiv(ID, loc, count, v);
            } else {
                static_assert(always_false<T>, "Unsupported vector dimension");
            }
        } else if constexpr (std::is_same_v<T, float>) {
            if constexpr (dim == 1u) {
                glProgramUniform1fv(ID, loc, count, v);
            } else if constexpr (dim == 2u) {
                glProgramUniform2fv(ID, loc, count, v);
            } else if constexpr (dim == 3u) {
                glProgramUniform3fv(ID, loc, count, v);
            } else if constexpr (dim == 4u) {
                glProgramUniform4fv(ID, loc, count, v);
            } else {
                static_assert(always_false<T>, "Unsupported vector dimension");
            }
        } else if constexpr (std::is_same_v<T, double>) {
            if constexpr (dim == 1u) {
                glProgramUniform1dv(ID, loc, count, v);
            } else if constexpr (dim == 2u) {
                glProgramUniform2dv(ID, loc, count, v);
            } else if constexpr (dim == 3u) {
                glProgramUniform3dv(ID, loc, count, v);
            } else if constexpr (dim == 4u) {
                glProgramUniform4dv(ID, loc, count, v);
            } else {
                static_assert(always_false<T>, "Unsupported vector dimension");
            }
        } else {
            static_assert(always_false<T>, "Unsupported uniform type");
        }
    }

    void dispatchCompute(const GLuint numGroupsX, const GLuint numGroupsY, const GLuint numGroupsZ, const GLbitfield barriers) const {
        glUseProgram(ID);
        glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
        glMemoryBarrier(barriers);
    }

    explicit inline operator bool() const noexcept {
        return ID != 0;
    }
};

#endif
