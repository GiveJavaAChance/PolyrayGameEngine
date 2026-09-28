#ifndef SCENENODE_H_INCLUDED
#define SCENENODE_H_INCLUDED

#pragma once

#include <cstdint>
#include <string>

#include <structure/DynamicArray.h>

struct SceneNode {
    std::string name;

    uint32_t entityID;

    uint32_t parent;
    DynamicArray<uint32_t> children;
};

#endif
