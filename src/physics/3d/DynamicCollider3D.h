#ifndef DYNAMICCOLLIDER3D_H_INCLUDED
#define DYNAMICCOLLIDER3D_H_INCLUDED

#include <cstdint>
#include <ecs/ComponentRef.h>
#include <physics/3d/Collider3D.h>
#include <physics/3d/PhysicsObject3D.h>

struct DynamicCollider3D {
    Collider3D impl;
    double offsetX, offsetY, offsetZ;
    double mass;
    ComponentRef<PhysicsObject3D> object;
};

template <>
struct ExportInfo<DynamicCollider3D> {
    constexpr static Export __export__[] = {
        {offsetof(DynamicCollider3D, offsetX), EXPORT_DVEC3, "Offset"},
        {offsetof(DynamicCollider3D, impl.sizeX), EXPORT_DVEC3, "Size"},
        {offsetof(DynamicCollider3D, impl.friction), EXPORT_DOUBLE, "Friction"},
        {offsetof(DynamicCollider3D, impl.restitution), EXPORT_DOUBLE, "Restitution"},
        {offsetof(DynamicCollider3D, mass), EXPORT_DOUBLE, "Mass"},
    };
};

#endif