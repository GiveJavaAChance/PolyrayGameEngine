#ifndef COLLIDERSHAPE2DMETADATA_H_INCLUDED
#define COLLIDERSHAPE2DMETADATA_H_INCLUDED

#pragma once

#include <cstdint>
#include <string_view>
#include <typereg.h>

struct ColliderShape2DMetadata {
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
    inline static ColliderShape2DMetadata get() {
        return ColliderShape2DMetadata{
            ColliderTypes::getTypeId<T>(),
            sizeof(T),
            alignof(T),
            type_name_of<T>()
        };
    }

    constexpr static ColliderShape2DMetadata invalid() {
        return ColliderShape2DMetadata{UINT32_MAX, 0u, 0u, ""};
    }
};

#endif