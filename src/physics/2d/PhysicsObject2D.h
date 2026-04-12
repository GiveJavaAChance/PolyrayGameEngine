#ifndef PHYSICSOBJECT2D_H_INCLUDED
#define PHYSICSOBJECT2D_H_INCLUDED

#pragma once

#include <ecs/Storage.h>

struct PhysicsObject2D {
    double posX, posY;
    double prevPosX, prevPosY;
    double accX, accY;
};

template<>
struct StorageInfo<PhysicsObject2D> {
    static constexpr StorageDataLayout layout = StorageDataLayout::CUSTOM;
};

#endif