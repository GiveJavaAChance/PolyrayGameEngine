#ifndef RAYHIT3D_H_INCLUDED
#define RAYHIT3D_H_INCLUDED

#pragma once

#include <prvl.h>

struct RayHit3D {
    float distance = 0.0f;
    float depth = 0.0f;
    vec3 normal = prvl::vec3();
    bool didHit = false;
};

#endif