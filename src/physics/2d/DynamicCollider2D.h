#ifndef DYNAMICCOLLIDER2D_H_INCLUDED
#define DYNAMICCOLLIDER2D_H_INCLUDED

#include <cstdint>
#include <ecs/ComponentRef.h>
#include <physics/2d/Collider2D.h>
#include <physics/2d/PhysicsObject2D.h>

struct DynamicCollider2D {
    Collider2D impl;
    double offsetX, offsetY;
    ComponentRef<PhysicsObject2D> object;
};

#endif
