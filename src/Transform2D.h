#ifndef TRANSFORM2D_H_INCLUDED
#define TRANSFORM2D_H_INCLUDED

#pragma once

#include <World.h>
#include <ecs/Export.h>
#include <prvl.h>
#include <serial/Serial.h>

struct Transform2D {
    mat3 global;
    mat3 local;
    bool dirtyGlobal;
    bool dirtyLocal;

    Transform2D(const mat3& tx) : global(diag(prvl::vec3(1.0f))), local(tx), dirtyGlobal(false), dirtyLocal(true) {
    }

    Transform2D() : Transform2D(diag(prvl::vec3(1.0f))) {
    }
};

template <>
struct Serial<Transform2D> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        output.write(world->ecs.getPtr<Transform2D>(componentID)->local);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        e.addComponent(Transform2D{input.read<mat3>()});
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
