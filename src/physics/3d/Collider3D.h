#ifndef COLLIDER3D_H_INCLUDED
#define COLLIDER3D_H_INCLUDED

#pragma once

#include <ecs/Export.h>
#include <prvl.h>

struct Collider3D {
    uint32_t typeId;
    void* userData;
    dvec3 pos;
    dvec3 size;
    double friction, restitution;
};

template <>
struct ExportInfo<Collider3D> {
    constexpr static Export __export__[] = {
        {offsetof(Collider3D, pos), EXPORT_DVEC3, "Position"},
        {offsetof(Collider3D, size), EXPORT_DVEC3, "Size"},
        {offsetof(Collider3D, friction), EXPORT_DOUBLE, "Friction"},
        {offsetof(Collider3D, restitution), EXPORT_DOUBLE, "Restitution"},
    };
};

#endif