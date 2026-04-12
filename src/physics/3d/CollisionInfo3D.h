#ifndef COLLISIONINFO3D_H_INCLUDED
#define COLLISIONINFO3D_H_INCLUDED

#pragma once

struct CollisionInfo3D {
    double collisionNormalX;
    double collisionNormalY;
    double collisionNormalZ;
    double penetrationDepth;
};

#endif