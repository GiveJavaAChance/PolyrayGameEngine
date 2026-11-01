#ifndef COLLISIONINFO2D_H_INCLUDED
#define COLLISIONINFO2D_H_INCLUDED

#pragma once

#include <prvl.h>

struct CollisionInfo2D {
    dvec2 collisionNormal;
    double penetrationDepth;
};

#endif