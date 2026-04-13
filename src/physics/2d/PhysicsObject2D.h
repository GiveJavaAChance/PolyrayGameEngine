#ifndef PHYSICSOBJECT2D_H_INCLUDED
#define PHYSICSOBJECT2D_H_INCLUDED

#pragma once

#include <ecs/Storage.h>
#include <structure/MultiDynamicArray.h>
#include <structure/Registry.h>

struct PhysicsObject2D {
    double posX, posY;
    double prevPosX, prevPosY;
    double accX, accY;
};

template<>
struct StorageInfo<PhysicsObject2D> {
    static constexpr StorageDataLayout layout = StorageDataLayout::CUSTOM;
};

template<>
struct Storage<PhysicsObject2D, StorageDataLayout::CUSTOM> {
    MultiDynamicArray<double, double, double, double, double, double> objects;
    Registry reg;

    inline uint32_t add(const PhysicsObject2D& component) noexcept {
        objects.add(
            component.posX, component.posY,
            component.prevPosX, component.prevPosY,
            component.accX, component.accY
        );
        return reg.create();
    }

    inline void set(uint32_t componentID, const PhysicsObject2D& component) noexcept {
        objects.setIndex(reg[componentID],
            component.posX, component.posY,
            component.prevPosX, component.prevPosY,
            component.accX, component.accY
        );
    }

    inline void remove(uint32_t componentID) noexcept {
        uint32_t loc;
        if(reg.remove(componentID, loc)) {
            objects.set(loc, objects, objects.size() - 1u);
        }
        objects.removeEnd();
    }

    inline PhysicsObject2D get(uint32_t id) const noexcept {
        uint32_t loc = reg[id];
        return PhysicsObject2D{
            objects.column<0>()[loc],
            objects.column<1>()[loc],
            objects.column<2>()[loc],
            objects.column<3>()[loc],
            objects.column<4>()[loc],
            objects.column<5>()[loc]
        };
    }
};

#endif