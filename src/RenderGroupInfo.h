#ifndef RENDERGROUPINFO_H_INCLUDED
#define RENDERGROUPINFO_H_INCLUDED

#pragma once

#include <utils/member_offset.h>

#include <MaterialType.h>
#include <SkinInstance.h>
#include <Transform3D.h>

struct RenderGroupInfo {
    MaterialType materialType;
    uint32_t componentType;
    uint32_t memberOffset;
    uint32_t memberSize;

    template <Component T, typename V, V T::* Member>
    static RenderGroupInfo create(const MaterialType& materialType) {
        return RenderGroupInfo{materialType, ComponentMetadata::typeOf<T>(), member_offset<T, V, Member>(), sizeof(V)};
    }
};

#endif