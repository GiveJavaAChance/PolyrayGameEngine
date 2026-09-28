#ifndef SKININSTANCE_H_INCLUDED
#define SKININSTANCE_H_INCLUDED

#pragma once

#include <cstdint>

#include <World.h>
#include <prvl.h>

struct SkinInstance {
    uint32_t skinID;

    uint32_t transformID;

    struct {
        uint32_t skinIdx;
        mat4 tx;
    } data;
};

template <>
struct Serial<SkinInstance> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        SkinInstance* r = world->ecs.getPtr<SkinInstance>(componentID);
        output.write(r->skinID);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        SkinInstance r{};
        input.read(r.skinID);
        e.addComponent(r);
    }
};

template <>
struct ExportInfo<SkinInstance> {
    constexpr static Export __export__[] = {
        {offsetof(SkinInstance, skinID), EXPORT_UINT, "Skin ID"},
    };
};

#endif