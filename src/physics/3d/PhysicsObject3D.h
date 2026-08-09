#ifndef PHYSICSOBJECT3D_H_INCLUDED
#define PHYSICSOBJECT3D_H_INCLUDED

#pragma once

#include <ecs/Component.h>
#include <ecs/Storage.h>
#include <structure/MultiDynamicArray.h>
#include <structure/Registry.h>

struct PhysicsObject3D {
    double posX, posY, posZ;
    double prevPosX, prevPosY, prevPosZ;
    double accX, accY, accZ;
};

template <>
struct ExportInfo<PhysicsObject3D> {
    constexpr static Export __export__[] = {
        {offsetof(PhysicsObject3D, posX), EXPORT_DVEC3, "Position"},
        {offsetof(PhysicsObject3D, prevPosX), EXPORT_DVEC3, "Previous Position"},
        {offsetof(PhysicsObject3D, accX), EXPORT_DVEC3, "Acceleration"},
    };
};

template <>
struct Storage<PhysicsObject3D> {
    MultiDynamicArray<double, double, double, double, double, double, double, double, double> objects;
    Registry reg;

    inline uint32_t add(const PhysicsObject3D& component) noexcept {
        objects.add(
            component.posX, component.posY, component.posZ,
            component.prevPosX, component.prevPosY, component.prevPosZ,
            component.accX, component.accY, component.accZ
        );
        return reg.create();
    }

    inline void set(uint32_t componentID, const PhysicsObject3D& component) noexcept {
        objects.setIndex(reg[componentID],
            component.posX, component.posY, component.posZ,
            component.prevPosX, component.prevPosY, component.prevPosZ,
            component.accX, component.accY, component.accZ
        );
    }

    inline void remove(uint32_t componentID) noexcept {
        uint32_t loc;
        if (reg.remove(componentID, loc)) {
            objects.set(loc, objects, objects.size() - 1u);
        }
        objects.removeEnd();
    }

    inline bool valid(uint32_t componentID) {
        return reg.valid(componentID);
    }

    inline PhysicsObject3D get(uint32_t id) const noexcept {
        uint32_t loc = reg[id];
        return PhysicsObject3D{
            objects.column<0>()[loc],
            objects.column<1>()[loc],
            objects.column<2>()[loc],
            objects.column<3>()[loc],
            objects.column<4>()[loc],
            objects.column<5>()[loc],
            objects.column<6>()[loc],
            objects.column<7>()[loc],
            objects.column<8>()[loc]
        };
    }
};

#endif