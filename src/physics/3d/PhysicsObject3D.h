#ifndef PHYSICSOBJECT3D_H_INCLUDED
#define PHYSICSOBJECT3D_H_INCLUDED

#pragma once

#include <ecs/Storage.h>

struct PhysicsObject3D {
    double posX, posY, posZ;
    double prevPosX, prevPosY, prevPosZ;
    double accX, accY, accZ;
};

template<>
struct StorageInfo<PhysicsObject3D> {
    static constexpr StorageDataLayout layout = StorageDataLayout::CUSTOM;
};

#endif