#ifndef SCENE3D_H_INCLUDED
#define SCENE3D_H_INCLUDED

#pragma once

#include <cstdint>

#include <structure/UnorderedRegistry.h>

struct ECS;

template<Component T>
struct ComponentRef;
struct Node3D;
struct SceneNode3D;

struct Scene3D {
private:
    UnorderedRegistry<SceneNode3D> nodes;
    uint32_t root;

    ECS* ecs;

    void updateNode(uint32_t node, Node3D* nodeData, bool dirty);

    void disconnectFromParent(uint32_t node);

    void removeNodes(uint32_t node);

public:
    Scene3D(ECS* ecs);

    inline uint32_t getRootNode();

    void setRootNode(ComponentRef<Node3D> node);

    uint32_t addNode(uint32_t parent, ComponentRef<Node3D> node);

    void removeNode(uint32_t node);

    ComponentRef<Node3D> getChild(uint32_t node, uint32_t index);

    void setParent(uint32_t node, uint32_t newParent, bool rebase = true);

    void update();
};

#endif
