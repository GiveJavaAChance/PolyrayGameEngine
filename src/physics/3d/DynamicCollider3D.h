#ifndef DYNAMICCOLLIDER3D_H_INCLUDED
#define DYNAMICCOLLIDER3D_H_INCLUDED

#pragma once

#include <physics/3d/Collider3D.h>
#include <physics/3d/PhysicsObject3D.h>

struct DynamicCollider3D {
    Collider3D impl;
    dvec3 offset;
    double mass;
    uint32_t objectID;
};

template <>
struct ExportInfo<DynamicCollider3D> {
    constexpr static Export __export__[] = {
        {offsetof(DynamicCollider3D, offset), EXPORT_DVEC3, "Offset"},
        {offsetof(DynamicCollider3D, impl.size), EXPORT_DVEC3, "Size"},
        {offsetof(DynamicCollider3D, impl.friction), EXPORT_DOUBLE, "Friction"},
        {offsetof(DynamicCollider3D, impl.restitution), EXPORT_DOUBLE, "Restitution"},
        {offsetof(DynamicCollider3D, mass), EXPORT_DOUBLE, "Mass"},
    };
};

#endif