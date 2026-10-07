#ifndef PHYSICSRAYHIT2D_H_INCLUDED
#define PHYSICSRAYHIT2D_H_INCLUDED

#pragma once

#include <prvl.h>

struct PhysicsRayHit2D {
    float distance = 0.0f;
    float depth = 0.0f;
    vec2 normal = prvl::vec2();
    bool didHit = false;

    uint32_t entityID = UINT32_MAX;
    uint32_t colliderID = UINT32_MAX;
    bool isDynamic = false;
};

#endif