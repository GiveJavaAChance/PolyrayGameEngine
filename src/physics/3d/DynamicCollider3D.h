#ifndef DYNAMICCOLLIDER3D_H_INCLUDED
#define DYNAMICCOLLIDER3D_H_INCLUDED

#include <cstdint>
#include <ecs/ComponentRef.h>
#include <physics/3d/Collider3D.h>
#include <physics/3d/PhysicsObject3D.h>

struct DynamicCollider3D {
    ComponentRef<PhysicsObject3D> object;
    Collider3D impl;
    double offsetX, offsetY, offsetZ;
};

#endif