#ifndef COLLIDER3D_H_INCLUDED
#define COLLIDER3D_H_INCLUDED

#pragma once

#include <cstdint>

#include <ecs/Export.h>

struct Collider3D {
    uint32_t typeId;
    void* userData;
    double posX, posY, posZ;
    double sizeX, sizeY, sizeZ;
    double friction, restitution;
};

template <>
struct ExportInfo<Collider3D> {
    constexpr static Export __export__[] = {
        {offsetof(Collider3D, posX), EXPORT_DVEC3, "Position"},
        {offsetof(Collider3D, sizeX), EXPORT_DVEC3, "Size"},
        {offsetof(Collider3D, friction), EXPORT_DOUBLE, "Friction"},
        {offsetof(Collider3D, restitution), EXPORT_DOUBLE, "Restitution"},
    };
};

#endif