#ifndef SCENE2D_H_INCLUDED
#define SCENE2D_H_INCLUDED

#pragma once

#include <cstdint>

#include <structure/UnorderedRegistry.h>

struct ECS;

template<Component T>
struct ComponentRef;
struct Node2D;
struct SceneNode2D;

struct Scene2D {
private:
    UnorderedRegistry<SceneNode2D> nodes;
    uint32_t root;

    ECS* ecs;

    void updateNode(uint32_t node, Node2D* nodeData, bool dirty);

    void disconnectFromParent(uint32_t node);

    void removeNodes(uint32_t node);

public:
    Scene2D(ECS* ecs);

    inline uint32_t getRootNode();

    void setRootNode(ComponentRef<Node2D> node);

    uint32_t addNode(uint32_t parent, ComponentRef<Node2D> node);

    void removeNode(uint32_t node);

    ComponentRef<Node2D> getChild(uint32_t node, uint32_t index);

    void setParent(uint32_t node, uint32_t newParent, bool rebase = true);

    void frameUpdate(double dt);
};

#endif
