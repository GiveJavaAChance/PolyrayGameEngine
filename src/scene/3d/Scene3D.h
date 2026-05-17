#ifndef SCENE3D_H_INCLUDED
#define SCENE3D_H_INCLUDED

#pragma once

#include <cstdint>

#include <structure/UnorderedRegistry.h>

struct ECS;
struct Entity;

struct SceneNode;
struct Transform3D;

struct Scene3D {
private:
    UnorderedRegistry<SceneNode> nodes;
    uint32_t root;

    ECS* ecs;

    void updateNode(uint32_t node, Transform3D* nodeData, bool dirty);

    void disconnectFromParent(uint32_t node);

    void removeNodes(uint32_t node);

public:
    Scene3D(ECS* ecs);

    inline uint32_t getRootNode() const;

    uint32_t setRootNode(Entity e);

    uint32_t addNode(uint32_t parent, Entity e);

    void removeNode(uint32_t node);

    Entity getChild(uint32_t node, uint32_t index);

    void setParent(uint32_t node, uint32_t newParent, bool rebase = true);

    inline Entity getNode(uint32_t node);

    void frameUpdate(double dt);
};

#endif
