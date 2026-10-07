#ifndef PHYSICSOBJECT2D_H_INCLUDED
#define PHYSICSOBJECT2D_H_INCLUDED

#include "structure/DynamicArray.h"
#pragma once

#include <ecs/Export.h>
#include <ecs/Storage.h>
#include <structure/MultiDynamicArray.h>
#include <structure/Registry.h>

struct PhysicsObject2D {
    double posX, posY;
    double prevPosX, prevPosY;
    double accX, accY;
    uint32_t transformID;
};

template <>
struct ExportInfo<PhysicsObject2D> {
    constexpr static Export __export__[] = {
        {offsetof(PhysicsObject2D, posX), EXPORT_DVEC2, "Position"},
        {offsetof(PhysicsObject2D, prevPosX), EXPORT_DVEC2, "Previous Position"},
        {offsetof(PhysicsObject2D, accX), EXPORT_DVEC2, "Acceleration"},
    };
};

template <>
struct Storage<PhysicsObject2D> {
    Registry reg;
    DynamicArray<uint32_t> entitySet;

    MultiDynamicArray<double, double, double, double, double, double, uint32_t> objects;

    inline uint32_t add(PhysicsObject2D&& component, uint32_t entityID) noexcept {
        objects.add(
            component.posX, component.posY,
            component.prevPosX, component.prevPosY,
            component.accX, component.accY,
            component.transformID);
        entitySet.add(entityID);
        return reg.create();
    }

    inline void set(uint32_t componentID, PhysicsObject2D&& component) noexcept {
        objects.setIndex(reg[componentID],
            component.posX, component.posY,
            component.prevPosX, component.prevPosY,
            component.accX, component.accY,
            component.transformID);
    }

    inline void remove(uint32_t componentID) noexcept {
        uint32_t loc;
        if (reg.remove(componentID, loc)) {
            uint32_t end = objects.size() - 1u;
            objects.set(loc, objects, end);
            entitySet[loc] = entitySet[end];
        }
        objects.removeEnd();
        entitySet.removeEnd(1u);
    }

    inline bool valid(uint32_t componentID) {
        return reg.valid(componentID);
    }

    inline PhysicsObject2D get(uint32_t id) const noexcept {
        uint32_t loc = reg[id];
        return PhysicsObject2D{
            objects.column<0>()[loc],
            objects.column<1>()[loc],
            objects.column<2>()[loc],
            objects.column<3>()[loc],
            objects.column<4>()[loc],
            objects.column<5>()[loc],
            objects.column<6>()[loc],
        };
    }
};

#endif