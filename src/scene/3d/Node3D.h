#ifndef NODE3D_H_INCLUDED
#define NODE3D_H_INCLUDED

#pragma once

#include <ecs/ECS.h>
#include <prvl.h>

struct Node3D {
    Entity e;
    mat4 local;
    mat4 global;
    bool dirtyLocal;
    bool dirtyGlobal;
};

#endif
