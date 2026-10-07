#ifndef RAYHIT2D_H_INCLUDED
#define RAYHIT2D_H_INCLUDED

#pragma once

#include <prvl.h>

struct RayHit2D {
    float distance = 0.0f;
    float depth = 0.0f;
    vec2 normal = prvl::vec2();
    bool didHit = false;
};

#endif