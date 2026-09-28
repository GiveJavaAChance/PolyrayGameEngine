#ifndef SHAPE_H_INCLUDED
#define SHAPE_H_INCLUDED

#pragma once

#include <cstdint>
#include <iostream>
#include <type_traits>

#include <prvl.h>
#include <Mesh.h>

struct GeometryVertex {
    vec3 position;
    vec3 normal;
    vec3 tangent;
    vec2 uv;
    vec4 color;
};

enum FaceDirection : uint32_t {
    NEGATIVE_X = 0u,
    POSITIVE_X = 1u,
    NEGATIVE_Y = 2u,
    POSITIVE_Y = 3u,
    NEGATIVE_Z = 4u,
    POSITIVE_Z = 5u
};
struct Shape {
private:
    constexpr static GeometryVertex CUBE_VERTICES[]{
        {{0.0f, 0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{0.0f, 0.0f, 1.0f}, {-1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{0.0f, 1.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{0.0f, 1.0f, 1.0f}, {-1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},

        {{1.0f, 0.0f, 1.0f}, { 1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{1.0f, 0.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{1.0f, 1.0f, 1.0f}, { 1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},

        {{1.0f, 0.0f, 1.0f}, { 0.0f, -1.0f,  0.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{0.0f, 0.0f, 1.0f}, { 0.0f, -1.0f,  0.0f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{1.0f, 0.0f, 0.0f}, { 0.0f, -1.0f,  0.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{0.0f, 0.0f, 0.0f}, { 0.0f, -1.0f,  0.0f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},

        {{0.0f, 1.0f, 1.0f}, { 0.0f,  1.0f,  0.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{1.0f, 1.0f, 1.0f}, { 0.0f,  1.0f,  0.0f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{0.0f, 1.0f, 0.0f}, { 0.0f,  1.0f,  0.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f}, { 0.0f,  1.0f,  0.0f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},

        {{1.0f, 0.0f, 0.0f}, { 0.0f,  0.0f, -1.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{0.0f, 0.0f, 0.0f}, { 0.0f,  0.0f, -1.0f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f}, { 0.0f,  0.0f, -1.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{0.0f, 1.0f, 0.0f}, { 0.0f,  0.0f, -1.0f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},

        {{0.0f, 0.0f, 1.0f}, { 0.0f,  0.0f,  1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{1.0f, 0.0f, 1.0f}, { 0.0f,  0.0f,  1.0f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{0.0f, 1.0f, 1.0f}, { 0.0f,  0.0f,  1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{1.0f, 1.0f, 1.0f}, { 0.0f,  0.0f,  1.0f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
    };
    constexpr static uint32_t CUBE_INDICES[]{
        0u,  1u,  2u,  1u,  3u,  2u,
        4u,  5u,  6u,  5u,  7u,  6u,
        8u,  9u,  10u, 9u,  11u, 10u,
        12u, 13u, 14u, 13u, 15u, 14u,
        16u, 17u, 18u, 17u, 19u, 18u,
        20u, 21u, 22u, 21u, 23u, 22u,
    };

public:
    template <typename V>
    constexpr static Mesh<V> box(const vec3& min, const vec3& max) {
        Mesh<V> mesh;
        mesh.vertices.ensureCapacity(24u);
        for (uint32_t i = 0u; i < 24u; i++) {
            const GeometryVertex& v = CUBE_VERTICES[i];
            V vert{};
            if constexpr (HasPosition<V>) {
                vert.position = mix(min, max, v.position);
            }
            if constexpr (HasNormal<V>) {
                vert.normal = v.normal;
            }
            if constexpr (HasTangent<V>) {
                vert.tangent = v.tangent;
            }
            if constexpr (HasUv<V>) {
                vert.uv = v.uv;
            }
            if constexpr (HasColor<V>) {
                vert.color = v.color;
            }
            mesh.vertices.add(vert);
        }
        mesh.indices.addAll(CUBE_INDICES, 36u);
        return mesh;
    }

    template <typename V>
    constexpr static Mesh<V> box(const vec3& min, const vec3& max, V (*remap)(const GeometryVertex&)) {
        Mesh<V> mesh;
        mesh.vertices.ensureCapacity(24u);
        for (uint32_t i = 0u; i < 24u; i++) {
            GeometryVertex v = CUBE_VERTICES[i];
            v.position = mix(min, max, v.position);
            mesh.vertices.add(remap(v));
        }
        mesh.indices.addAll(CUBE_INDICES, 36u);
        return mesh;
    }

    template <typename V>
    constexpr static Mesh<V> plane(FaceDirection dir, const vec2& min, const vec2& max) {
        Mesh<V> mesh;
        mesh.vertices.ensureCapacity(4u);
        const GeometryVertex& faceVert = CUBE_VERTICES[dir * 4u];
        vec3 x = faceVert.tangent;
        vec3 y = cross(faceVert.normal, x);
        mat2x3 mat = prvl::mat2x3(x, y);
        for (uint32_t i = 0u; i < 4u; i++) {
            const GeometryVertex& v = CUBE_VERTICES[i + dir * 4u];
            V vert{};
            if constexpr (HasPosition<V>) {
                vert.position = mat * mix(min, max, v.uv);
            }
            if constexpr (HasNormal<V>) {
                vert.normal = v.normal;
            }
            if constexpr (HasTangent<V>) {
                vert.tangent = v.tangent;
            }
            if constexpr (HasUv<V>) {
                vert.uv = v.uv;
            }
            if constexpr (HasColor<V>) {
                vert.color = v.color;
            }
            mesh.vertices.add(vert);
        }
        mesh.indices.addAll(CUBE_INDICES, 6u);
        return mesh;
    }

    template <typename V>
    constexpr static Mesh<V> plane(FaceDirection dir, const vec2& min, const vec2& max, V (*remap)(const GeometryVertex&)) {
        Mesh<V> mesh;
        mesh.vertices.ensureCapacity(4u);
        const GeometryVertex& faceVert = CUBE_VERTICES[dir * 4u];
        vec3 x = faceVert.tangent;
        vec3 y = cross(faceVert.normal, x);
        mat2x3 mat = prvl::mat2x3(x, y);
        for (uint32_t i = 0u; i < 4u; i++) {
            GeometryVertex v = CUBE_VERTICES[i + dir * 4u];
            v.position = mat * mix(min, max, v.uv);
            mesh.vertices.add(remap(v));
        }
        mesh.indices.addAll(CUBE_INDICES, 6u);
        return mesh;
    }
};

#endif