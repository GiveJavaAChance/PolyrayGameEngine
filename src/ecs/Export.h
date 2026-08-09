#ifndef EXPORT_H_INCLUDED
#define EXPORT_H_INCLUDED

#pragma once

#include <concepts>
#include <cstdint>

enum ExportType : uint8_t {
    EXPORT_BOOL,
    EXPORT_INT,
    EXPORT_IVEC2,
    EXPORT_IVEC3,
    EXPORT_IVEC4,
    EXPORT_UINT,
    EXPORT_UVEC2,
    EXPORT_UVEC3,
    EXPORT_UVEC4,
    EXPORT_FLOAT,
    EXPORT_VEC2,
    EXPORT_VEC3,
    EXPORT_VEC4,
    EXPORT_MAT2,
    EXPORT_MAT3,
    EXPORT_MAT4,
    EXPORT_COLOR_RGB,
    EXPORT_COLOR_RGBA,
    EXPORT_DOUBLE,
    EXPORT_DVEC2,
    EXPORT_DVEC3,
    EXPORT_DVEC4,

    EXPORT_TEXTURE,

    EXPORT_DIRTY_FLAG
};

struct Export {
    size_t offset;
    ExportType type;
    const char* name;
};

template <typename T>
struct ExportInfo {
};

template <typename T>
concept HasExport = requires { ExportInfo<T>::__export__; } && std::same_as<std::remove_cv_t<std::remove_extent_t<std::remove_reference_t<decltype(ExportInfo<T>::__export__)>>>, Export>;

#endif