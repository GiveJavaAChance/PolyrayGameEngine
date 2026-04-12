#ifndef SCENENODE2D_H_INCLUDED
#define SCENENODE2D_H_INCLUDED

#pragma once

#include <cstdint>
#include <scene/2d/Node2D.h>
#include <structure/DynamicArray.h>

struct SceneNode2D {
    uint32_t id;
    ComponentRef<Node2D> ref;

    uint32_t parent;
    DynamicArray<uint32_t> children;
};

#endif
