#ifndef TRANSFORM3D_H_INCLUDED
#define TRANSFORM3D_H_INCLUDED

#pragma once

#include <prvl.h>

struct Transform3D {
    mat4 global;
    mat4 local;
    bool dirtyGlobal;
    bool dirtyLocal;
};

#endif