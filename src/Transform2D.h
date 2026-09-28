#ifndef TRANSFORM2D_H_INCLUDED
#define TRANSFORM2D_H_INCLUDED

#pragma once

#include <World.h>
#include <ecs/Export.h>
#include <prvl.h>
#include <serial/Serial.h>

struct Transform2D {
    vec2 position;
    float rotation;
    vec2 scale;

    mat3 local;
    mat3 global;

    bool dirtyTRS;
    bool dirtyLocal;
    bool dirtyGlobal;

    Transform2D(const vec2& position = prvl::vec2(), float rotation = 0.0f, const vec2& scale = prvl::vec2(1.0f)) : position(position), rotation(rotation), scale(scale), local(diag(prvl::vec3(1.0f))), global(diag(prvl::vec3(1.0f))), dirtyTRS(true), dirtyLocal(false), dirtyGlobal(false) {
    }
};

template <>
struct Serial<Transform2D> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        Transform2D* tx = world->ecs.getPtr<Transform2D>(componentID);
        output.write(tx->position);
        output.write(tx->rotation);
        output.write(tx->scale);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        e.addComponent(Transform2D{input.read<vec2>(), input.read<float>(), input.read<vec2>()});
    }
};

template <>
struct ExportInfo<Transform2D> {
    constexpr static Export __export__[] = {
        {offsetof(Transform2D, local), EXPORT_MAT3, ""},
        {offsetof(Transform2D, dirtyLocal), EXPORT_DIRTY_FLAG, ""},
    };
};

#endif
