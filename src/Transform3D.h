#ifndef TRANSFORM3D_H_INCLUDED
#define TRANSFORM3D_H_INCLUDED

#pragma once

#include <World.h>
#include <ecs/Export.h>
#include <prvl.h>
#include <serial/Serial.h>

struct Transform3D {
    mat4 global;
    mat4 local;
    bool dirtyGlobal;
    bool dirtyLocal;

    Transform3D(const mat4& tx) : global(diag(prvl::vec4(1.0f))), local(tx), dirtyGlobal(false), dirtyLocal(true) {
    }

    Transform3D() : Transform3D(diag(prvl::vec4(1.0f))) {
    }
};

template <>
struct Serial<Transform3D> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        output.write(world->ecs.getPtr<Transform3D>(componentID)->local);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        e.addComponent(Transform3D{input.read<mat4>()});
    }
};

template <>
struct ExportInfo<Transform3D> {
    constexpr static Export __export__[] = {
        {offsetof(Transform3D, local), EXPORT_MAT4, ""},
        {offsetof(Transform3D, dirtyLocal), EXPORT_DIRTY_FLAG, ""},
    };
};

#endif
