#ifndef RENDERGROUPINFO_H_INCLUDED
#define RENDERGROUPINFO_H_INCLUDED

#include <type_traits>
#pragma once

#include <utils/member_offset.h>

#include <MaterialType.h>
#include <RenderRequirements.h>
#include <SkinInstance.h>
#include <Transform3D.h>

struct RenderGroupInfo {
    MaterialType materialType;
    uint32_t componentType;
    uint32_t memberOffset;
    uint32_t memberSize;

    RenderRequirements requirements;

    template <typename T, auto Member>
    static RenderGroupInfo create(const MaterialType& materialType, const RenderRequirements& requirements = {}) {
        using V = std::remove_reference_t<decltype(std::declval<T>().*Member)>;
        return RenderGroupInfo{
            materialType,
            ComponentMetadata::typeOf<T>(),
            member_offset<T, V, Member>(),
            sizeof(V),
            requirements,
        };
    }

    static RenderGroupInfo staticPBR();
    static RenderGroupInfo skinnedPBR();
    static RenderGroupInfo maskedStaticPBR();
    static RenderGroupInfo maskedSkinnedPBR();
};

#endif