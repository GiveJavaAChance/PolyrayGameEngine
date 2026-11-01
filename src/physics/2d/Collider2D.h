#ifndef COLLIDER2D_H_INCLUDED
#define COLLIDER2D_H_INCLUDED

#pragma once

#include <ecs/Export.h>
#include <prvl.h>

struct Collider2D {
    uint32_t typeId;
    void* userData;
    dvec2 pos;
    dvec2 size;
    double friction, restitution;
};

template <>
struct ExportInfo<Collider2D> {
    constexpr static Export __export__[] = {
        {offsetof(Collider2D, pos), EXPORT_DVEC2, "Position"},
        {offsetof(Collider2D, size), EXPORT_DVEC2, "Size"},
        {offsetof(Collider2D, friction), EXPORT_DOUBLE, "Friction"},
        {offsetof(Collider2D, restitution), EXPORT_DOUBLE, "Restitution"},
    };
};

#endif