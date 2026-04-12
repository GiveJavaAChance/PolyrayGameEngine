#ifndef RESOURCEMANAGER_H_INCLUDED
#define RESOURCEMANAGER_H_INCLUDED

#pragma once

#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <windows.h>
#include <immintrin.h>
#include <stb_image.h>
#include <rendering/GLTexture.h>

enum ImageFormat : uint8_t {
    RGBA8,
    BYTE_GRAY,
    BINARY,
    FLOAT32_RGB
};

template<ImageFormat F>
struct ImageFormatType;

template<> struct ImageFormatType<ImageFormat::RGBA8> { using type = uint32_t; constexpr static int format = STBI_rgb_alpha; constexpr static GLenum glFormat = GL_RGBA8; constexpr static GLenum glPixFormat = GL_RGBA; constexpr static GLenum glType = GL_UNSIGNED_BYTE; };
template<> struct ImageFormatType<ImageFormat::BINARY> { using type = uint32_t; constexpr static int format = STBI_grey; constexpr static GLenum glFormat = GL_R8; constexpr static GLenum glPixFormat = GL_R; constexpr static GLenum glType = GL_UNSIGNED_BYTE; };
template<> struct ImageFormatType<ImageFormat::BYTE_GRAY> { using type = uint8_t; constexpr static int format = STBI_grey; constexpr static GLenum glFormat = GL_R8; constexpr static GLenum glPixFormat = GL_R; constexpr static GLenum glType = GL_UNSIGNED_BYTE; };
template<> struct ImageFormatType<ImageFormat::FLOAT32_RGB> { using type = float; constexpr static int format = STBI_rgb; constexpr static GLenum glFormat = GL_R11F_G11F_B10F; constexpr static GLenum glPixFormat = GL_RGB; constexpr static GLenum glType = GL_FLOAT; };

namespace ResourceManager {
    namespace Internal {
        inline std::vector<std::filesystem::path> resources;

        inline std::filesystem::path getRootDir() {
            char buffer[MAX_PATH];
            GetModuleFileNameA(nullptr, buffer, MAX_PATH);
            return std::filesystem::path(buffer).parent_path();
        }
    }

    using namespace Internal;

    inline void addResource(const char* path) {
        resources.emplace_back(path);
    }

    inline void addLocalResource(const char* relativePath) {
        resources.push_back(getRootDir() / relativePath);
    }

    inline std::filesystem::path getResourcePath(const char* name) {
        for (std::filesystem::path& dir : resources) {
            std::filesystem::path fullPath = dir / name;
            if (std::filesystem::exists(fullPath)) {
                return fullPath;
            }
        }
        return {};
    }

    inline std::ifstream getResource(const char* name) {
        for (std::filesystem::path& dir : resources) {
            std::filesystem::path fullPath = dir / name;
            if (std::filesystem::exists(fullPath)) {
                std::ifstream file(fullPath, std::ios::binary);
                if (file) {
                    return file;
                }
            }
        }
        std::cerr << "Resource not found: " << name << std::endl;
        return std::ifstream{};
    }

    inline std::string getResourceAsString(const char* name) {
        std::ifstream in = getResource(name);
        if (!in) {
            return {};
        }
        in.seekg(0, std::ios::end);
        size_t size = in.tellg();
        in.seekg(0, std::ios::beg);
        std::string content(size, '\0');
        in.read(&content[0], size);
        return content;
    }

    template<ImageFormat format = RGBA8>
    inline typename ImageFormatType<format>::type* getResourceAsImage(const char* name, uint32_t& width, uint32_t& height) {
        std::filesystem::path path = getResourcePath(name);
        int channels;
        if constexpr (format == FLOAT32_RGB) {
            return stbi_loadf(
                reinterpret_cast<const char*>(path.u8string().c_str()),
                reinterpret_cast<int*>(&width), reinterpret_cast<int*>(&height),
                &channels,
                3
            );
        } else {
            using PixelType = ImageFormatType<format>::type;
            PixelType* data = reinterpret_cast<PixelType*>(stbi_load(
                reinterpret_cast<const char*>(path.u8string().c_str()),
                reinterpret_cast<int*>(&width), reinterpret_cast<int*>(&height),
                &channels,
                ImageFormatType<format>::format
            ));
            if (!data) {
                return nullptr;
            }
            if constexpr (format == BINARY) {
                const uint32_t numBits = width * height;
                const uint32_t numWords = (numBits + 31u) >> 5u;
                uint32_t* newData = reinterpret_cast<uint32_t*>(_mm_malloc(numWords * sizeof(uint32_t), 32u));
                std::memset(newData, 0, numWords * sizeof(uint32_t));
                for(uint32_t i = 0u; i < numBits; i++) {
                    newData[i >> 5u] |= ((data[i] > 127u) ? 1u : 0u) << (i & 31u);
                }
                _mm_free(data);
                data = newData;
            }
            return data;
        }
    }

    template<ImageFormat fmt = RGBA8>
    inline GLTexture getResourceAsTexture(const char* name, const uint32_t mipLevels = 1u, const GLenum format = ImageFormatType<fmt>::glFormat) {
        uint32_t width, height;
        using PixelType = ImageFormatType<fmt>::type;
        PixelType* pixels = getResourceAsImage<fmt>(name, width, height);
        if(!pixels) {
            return {};
        }
        GLTexture tex = GLTexture::createTexture2D(width, height, format, mipLevels);
        tex.set2DTextureData(pixels, width, height, 0, 0, ImageFormatType<fmt>::glPixFormat, ImageFormatType<fmt>::glType);
        _mm_free(pixels);
        return tex;
    }
}

#endif
