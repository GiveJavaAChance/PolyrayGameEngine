#ifndef PHYSICSRAYHIT3D_H_INCLUDED
#define PHYSICSRAYHIT3D_H_INCLUDED

#pragma once

#include <prvl.h>

struct PhysicsRayHit3D {
    float distance = 0.0f;
    float depth = 0.0f;
    vec3 normal = prvl::vec3();
    bool didHit = false;

    uint32_t entityID = UINT32_MAX;
    uint32_t colliderID = UINT32_MAX;
    bool isDynamic = false;
};

#endif