#ifndef DYNAMICCOLLIDER2D_H_INCLUDED
#define DYNAMICCOLLIDER2D_H_INCLUDED

#include <cstdint>
#include <ecs/ComponentRef.h>
#include <physics/2d/Collider2D.h>
#include <physics/2d/PhysicsObject2D.h>

struct DynamicCollider2D {
    Collider2D impl;
    double offsetX, offsetY;
    double mass;
    ComponentRef<PhysicsObject2D> object;
};

template <>
struct ExportInfo<DynamicCollider2D> {
    constexpr static Export __export__[] = {
        {offsetof(DynamicCollider2D, offsetX), EXPORT_DVEC2, "Offset"},
        {offsetof(DynamicCollider2D, impl.sizeX), EXPORT_DVEC2, "Size"},
        {offsetof(DynamicCollider2D, impl.friction), EXPORT_DOUBLE, "Friction"},
        {offsetof(DynamicCollider2D, impl.restitution), EXPORT_DOUBLE, "Restitution"},
        {offsetof(DynamicCollider2D, mass), EXPORT_DOUBLE, "Mass"},
    };
};

#endif
