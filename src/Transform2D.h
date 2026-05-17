#ifndef TRANSFORM2D_H_INCLUDED
#define TRANSFORM2D_H_INCLUDED

#pragma once

#include <prvl.h>

struct Transform2D {
    mat3 global;
    mat3 local;
    bool dirtyGlobal;
    bool dirtyLocal;
};

#endif