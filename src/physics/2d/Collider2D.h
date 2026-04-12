#ifndef COLLIDER2D_H_INCLUDED
#define COLLIDER2D_H_INCLUDED

#pragma once

#include <cstdint>

struct Collider2D {
    uint32_t typeId;
    void* userData;
    double posX, posY;
    double sizeX, sizeY;
    double friction, restitution;
};

#endif