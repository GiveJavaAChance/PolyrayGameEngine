#ifndef TRANSFORM3D_H_INCLUDED
#define TRANSFORM3D_H_INCLUDED

#pragma once

#include <World.h>
#include <ecs/Export.h>
#include <prvl.h>
#include <serial/Serial.h>

struct Transform3D {
    vec3 position;
    quat rotation;
    vec3 scale;

    mat4 local;
    mat4 global;

    bool dirtyTRS;
    bool dirtyLocal;
    bool dirtyGlobal;

    Transform3D(const vec3& position = prvl::vec3(), const quat& rotation = prvl::quat(), const vec3& scale = prvl::vec3(1.0f)) : position(position), rotation(rotation), scale(scale), local(diag(prvl::vec4(1.0f))), global(diag(prvl::vec4(1.0f))), dirtyTRS(true), dirtyLocal(false), dirtyGlobal(false) {
    }
};

template <>
struct Serial<Transform3D> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        Transform3D* tx = world->ecs.getPtr<Transform3D>(componentID);
        output.write(tx->position);
        output.write(tx->rotation);
        output.write(tx->scale);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        e.addComponent(Transform3D{input.read<vec3>(), input.read<quat>(), input.read<vec3>()});
    }
};

template <>
struct ExportInfo<Transform3D> {
    constexpr static Export __export__[] = {
        {offsetof(Transform3D, position), EXPORT_VEC3, "Position"},
        {offsetof(Transform3D, rotation), EXPORT_QUAT, "Rotation"},
        {offsetof(Transform3D, scale), EXPORT_VEC3, "Scale"},
        {offsetof(Transform3D, dirtyTRS), EXPORT_DIRTY_FLAG, ""},
    };
};

#endif
