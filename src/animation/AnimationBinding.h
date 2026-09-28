#ifndef ANIMATIONBINDING_H_INCLUDED
#define ANIMATIONBINDING_H_INCLUDED

#pragma once

#include <cstdint>
#include <string>

#include <utils/member_offset.h>

#include <ecs/ECS.h>

struct AnimationBinding {
    std::string targetPath;

    uint32_t componentType;
    uint32_t memberOffset;
    uint32_t memberSize;

    uint32_t dirtyOffset;
    uint32_t dirtyMask = 0u;
    uint32_t dirtySize;

    template <typename T, typename V, V T::* Member>
    inline void setup() {
        componentType = ComponentMetadata::typeOf<T>();
        memberOffset = member_offset<T, V, Member>();
        memberSize = sizeof(V);
    }

    template <typename T, typename V, V T::* Member>
    inline void setDirtyFlagMember(uint32_t dirtyFlagMask) {
        dirtyOffset = member_offset<T, V, Member>();
        dirtyMask = dirtyFlagMask;
        dirtySize = sizeof(V);
    }
};

#endif