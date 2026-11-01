#ifndef COLLISIONINFO3D_H_INCLUDED
#define COLLISIONINFO3D_H_INCLUDED

#pragma once

#include <prvl.h>

struct CollisionInfo3D {
    dvec3 collisionNormal;
    double penetrationDepth;
};

#endif