#ifndef SCENENODE3D_H_INCLUDED
#define SCENENODE3D_H_INCLUDED

#pragma once

#include <cstdint>
#include <scene/3d/Node3D.h>
#include <structure/DynamicArray.h>

struct SceneNode3D {
    uint32_t id;
    ComponentRef<Node3D> ref;

    uint32_t parent;
    DynamicArray<uint32_t> children;
};

#endif
