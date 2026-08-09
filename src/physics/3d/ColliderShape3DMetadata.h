#ifndef COLLIDERSHAPE3DMETADATA_H_INCLUDED
#define COLLIDERSHAPE3DMETADATA_H_INCLUDED

#pragma once

#include <cstdint>
#include <string_view>
#include <typereg.h>

struct ColliderShape3DMetadata {
private:
    TYPE_REGISTRY(ColliderTypes)

public:
    uint32_t typeId;
    uint32_t size;
    uint32_t alignment;
    std::string_view name;

    template <typename T>
    inline static uint32_t typeOf() {
        return ColliderTypes::getTypeId<T>();
    }

    template <typename T>
    inline static ColliderShape3DMetadata get() {
        return ColliderShape3DMetadata{
            ColliderTypes::getTypeId<T>(),
            sizeof(T),
            alignof(T),
            type_name_of<T>()
        };
    }

    constexpr static ColliderShape3DMetadata invalid() {
        return ColliderShape3DMetadata{UINT32_MAX, 0u, 0u, ""};
    }
};

#endif