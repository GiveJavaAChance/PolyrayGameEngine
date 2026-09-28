#ifndef SCENE2D_H_INCLUDED
#define SCENE2D_H_INCLUDED

#pragma once

#include <cstdint>
#include <unordered_map>

#include <structure/UnorderedRegistry.h>

#include <scene/SceneNode.h>

struct World;
struct Entity;

struct Transform2D;

struct Scene2D {
private:
    UnorderedRegistry<SceneNode> nodes;
    std::unordered_map<uint32_t, uint32_t> entityMap;
    uint32_t root;

    World* world;

    void updateNode(uint32_t node, Transform2D* nodeData, bool dirty);

    void disconnectFromParent(uint32_t node);

    void removeNodes(uint32_t node);

public:
    Scene2D(World* world);

    uint32_t getRootNode() const;

    uint32_t setRootNode(const Entity& e, const std::string& name);

    uint32_t addNode(uint32_t parent, const Entity& e, const std::string& name);

    void removeNode(uint32_t node);

    uint32_t getChild(uint32_t node, uint32_t index);

    uint32_t getChildCount(uint32_t node);

    uint32_t getChild(uint32_t node, const char* name, uint32_t nameLength = UINT32_MAX);

    uint32_t getNode(uint32_t node, const char* path, uint32_t pathLength = UINT32_MAX);

    std::string& getNodeName(uint32_t node);

    std::string getNodePath(uint32_t node, uint32_t fromNode = UINT32_MAX);

    uint32_t getParent(uint32_t node);

    void setParent(uint32_t node, uint32_t newParent, bool rebase = true);

    Entity getEntity(uint32_t node);

    uint32_t getNode(uint32_t entityID);

    void frameUpdate(double dt);
};

#endif
