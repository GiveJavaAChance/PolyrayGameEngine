#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED

#pragma once

#include <structure/DynamicArray.h>

template <typename T>
concept HasPosition = requires(T v) { v.position; };

template <typename T>
concept HasNormal = requires(T v) { v.normal; };

template <typename T>
concept HasTangent = requires(T v) { v.tangent; };

template <typename T>
concept HasUv = requires(T v) { v.uv; };

template <typename T>
concept HasColor = requires(T v) { v.color; };

template <typename V>
struct Mesh {
    DynamicArray<V> vertices;
    DynamicArray<uint32_t> indices;
};

#endif