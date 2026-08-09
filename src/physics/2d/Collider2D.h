#ifndef COLLIDER2D_H_INCLUDED
#define COLLIDER2D_H_INCLUDED

#pragma once

#include <cstdint>

#include <utils/ByteIO.h>
#include <World.h>

struct Collider2D {
    uint32_t typeId;
    void* userData;
    double posX, posY;
    double sizeX, sizeY;
    double friction, restitution;
};

template <>
struct ExportInfo<Collider2D> {
    constexpr static Export __export__[] = {
        {offsetof(Collider2D, posX), EXPORT_DVEC2, "Position"},
        {offsetof(Collider2D, sizeX), EXPORT_DVEC2, "Size"},
        {offsetof(Collider2D, friction), EXPORT_DOUBLE, "Friction"},
        {offsetof(Collider2D, restitution), EXPORT_DOUBLE, "Restitution"},
    };
};

#endif