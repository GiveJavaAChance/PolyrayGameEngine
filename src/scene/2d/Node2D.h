#ifndef NODE2D_H_INCLUDED
#define NODE2D_H_INCLUDED

#pragma once

#include <ecs/ECS.h>
#include <prvl.h>

struct Node2D {
    Entity e;
    mat3 local;
    mat3 global;
    bool dirtyLocal;
    bool dirtyGlobal;
};

#endif
