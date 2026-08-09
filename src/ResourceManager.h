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
#include <scene/SceneData.h>
#include <ostream>

struct ResourcePath {
private:
    std::string str;

    void normalize() {
        for (char& c : str) {
            if (c == '\\') {
                c = '/';
            }
        }
    }

public:
    ResourcePath() {
    }

    ResourcePath(const char* path) : str(path) {
        normalize();
    }

    ResourcePath(const std::string& path) : str(path) {
        normalize();
    }

    const std::string& string() const {
        return str;
    }

    std::string getName() const {
        size_t pos = str.find_last_of('/');
        if (pos == std::string::npos) {
            return str;
        }
        return str.substr(pos + 1u);
    }

    std::string getExtension() const {
        size_t pos = str.find_last_of('.');
        if (pos == std::string::npos) {
            return "";
        }
        return str.substr(pos + 1u);
    }

    inline std::filesystem::path getAbsolutePath() const;

    bool isDirectory() const {
        return std::filesystem::is_directory(getAbsolutePath());
    }

    friend std::ostream& operator<<(std::ostream& out, const ResourcePath& path) {
        out.write(path.str.data(), path.str.size());
        return out;
    }
};

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
    inline std::filesystem::path getRootDir() {
        char buffer[MAX_PATH];
        GetModuleFileNameA(nullptr, buffer, MAX_PATH);
        return std::filesystem::path(buffer).parent_path();
    }

    inline DynamicArray<ResourcePath> listFiles(const ResourcePath& res) {
        std::filesystem::path path = res.getAbsolutePath();
        DynamicArray<ResourcePath> files;
        if (!std::filesystem::is_directory(path)) {
            return files;
        }
        size_t off = getRootDir().string().size() + 1ull;
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path)) {
            std::filesystem::path file = entry.path();
            files.emplace(file.string().substr(off));
        }
        return files;
    }

    inline DynamicArray<ResourcePath> find(const ResourcePath& res, const char* extension) {
        std::vector<std::filesystem::path> pathStack;
        pathStack.push_back(res.getAbsolutePath());
        std::string end = ".";
        end += extension;
        DynamicArray<ResourcePath> files;
        size_t off = getRootDir().string().size() + 1ull;
        while (pathStack.size() > 0u) {
            std::filesystem::path file = pathStack.back();
            pathStack.pop_back();
            if (std::filesystem::is_directory(file)) {
                for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(file)) {
                    pathStack.push_back(entry.path());
                }
            } else if (file.extension() == end) {
                files.emplace(file.string().substr(off));
            }
        }
        return files;
    }

    inline std::ifstream getResource(const ResourcePath& res) {
        std::filesystem::path fullPath = res.getAbsolutePath();
        if (std::filesystem::exists(fullPath)) {
            std::ifstream file(fullPath, std::ios::binary);
            if (file) {
                return file;
            }
        }
        std::cerr << "Resource not found: " << res << std::endl;
        return std::ifstream{};
    }

    inline std::string getResourceAsString(const ResourcePath& res) {
        std::ifstream in = getResource(res);
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
    inline typename ImageFormatType<format>::type* getResourceAsImage(const ResourcePath& res, uint32_t& width, uint32_t& height) {
        std::filesystem::path path = res.getAbsolutePath();
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
    inline GLTexture getResourceAsTexture(const ResourcePath& res, const uint32_t mipLevels = 1u, const GLenum format = ImageFormatType<fmt>::glFormat) {
        uint32_t width, height;
        using PixelType = ImageFormatType<fmt>::type;
        PixelType* pixels = getResourceAsImage<fmt>(res, width, height);
        if(!pixels) {
            return {};
        }
        GLTexture tex = GLTexture::createTexture2D(width, height, format, mipLevels);
        tex.set2DTextureData(pixels, width, height, 0, 0, ImageFormatType<fmt>::glPixFormat, ImageFormatType<fmt>::glType);
        _mm_free(pixels);
        return tex;
    }

    inline SceneData getResourceAsScene(const ResourcePath& res) {
        std::ifstream in = getResource(res);
        if (!in) {
            return {nullptr, 0u};
        }
        in.seekg(0, std::ios::end);
        size_t size = in.tellg();
        in.seekg(0, std::ios::beg);
        uint8_t* data = alloc<uint8_t>(size);
        in.read(reinterpret_cast<char*>(data), size);
        return {data, static_cast<uint32_t>(size)};
    }
}

inline std::filesystem::path ResourcePath::getAbsolutePath() const {
    return ResourceManager::getRootDir() / str;
}

#endif
