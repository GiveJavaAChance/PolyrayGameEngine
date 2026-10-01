#ifndef RENDERINSTANCE_H_INCLUDED
#define RENDERINSTANCE_H_INCLUDED

#pragma once

#include <cstdint>

#include <World.h>

struct RenderInstance {
    uint64_t objectID;

    uint32_t componentID = UINT32_MAX;

    RenderInstance(uint64_t objectID = UINT64_MAX) : objectID(objectID) {
    }
};

template <>
struct Serial<RenderInstance> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        RenderInstance* r = world->ecs.getPtr<RenderInstance>(componentID);
        output.write(r->objectID);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        RenderInstance r{};
        input.read(r.objectID);
        e.addComponent(r);
    }
};

template <>
struct ExportInfo<RenderInstance> {
    constexpr static Export __export__[] = {
        {offsetof(RenderInstance, objectID), EXPORT_UINT, "Object ID"},
    };
};

#endif
