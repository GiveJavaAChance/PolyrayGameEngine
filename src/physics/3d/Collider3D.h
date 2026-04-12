#ifndef COLLIDER3D_H_INCLUDED
#define COLLIDER3D_H_INCLUDED

#pragma once

#include <cstdint>

struct Collider3D {
    uint32_t typeId;
    void* userData;
    double posX, posY, posZ;
    double sizeX, sizeY, sizeZ;
    double friction, restitution;
};

#endif