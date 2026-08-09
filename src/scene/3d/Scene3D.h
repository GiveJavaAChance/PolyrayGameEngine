#ifndef SCENE3D_H_INCLUDED
#define SCENE3D_H_INCLUDED

#pragma once

#include <cstdint>
#include <unordered_map>

#include <structure/UnorderedRegistry.h>

#include <scene/SceneNode.h>

struct ECS;
struct Entity;

struct Transform3D;

struct Scene3D {
private:
    UnorderedRegistry<SceneNode> nodes;
    std::unordered_map<uint32_t, uint32_t> entityMap;
    uint32_t root;

    ECS* ecs;

    void updateNode(uint32_t node, Transform3D* nodeData, bool dirty);

    void disconnectFromParent(uint32_t node);

    void removeNodes(uint32_t node);

public:
    Scene3D(ECS* ecs);

    uint32_t getRootNode() const;

    uint32_t setRootNode(const Entity& e);

    uint32_t addNode(uint32_t parent, const Entity& e);

    void removeNode(uint32_t node);

    uint32_t getChild(uint32_t node, uint32_t index);

    uint32_t getChildCount(uint32_t node);

    uint32_t getParent(uint32_t node);

    void setParent(uint32_t node, uint32_t newParent, bool rebase = true);

    Entity getEntity(uint32_t node);

    uint32_t getNode(const Entity& e);

    void frameUpdate(double dt);
};

#endif
