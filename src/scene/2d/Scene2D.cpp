#include "Scene2D.h"

#include <Transform2D.h>
#include <World.h>
#include <scene/2d/Scene2DNodeUpdatedEvent.h>

#include <Profiler.h>

void Scene2D::updateNode(uint32_t node, Transform2D* nodeData, bool dirty) {
    const mat3& global = nodeData->global;
    ECS& ecs = world->ecs;
    SceneNode& sceneNode = nodes[node];
    if (dirty) {
        world->eventBus.fireDirect<Scene2DNodeUpdatedEvent>(node, sceneNode.entityID, nodeData);
    }
    for (uint32_t i = 0u; i < sceneNode.children.size(); i++) {
        uint32_t child = sceneNode.children[i];
        if (Transform2D* childData = ecs.getComponentPtr<Transform2D>(nodes[child].entityID)) {
            bool childDirty = dirty;
            if (childData->dirtyTRS) {
                childData->local = trsToMatrix(childData->position, childData->rotation, childData->scale);
                childData->dirtyTRS = false;
                childDirty = true;
            }
            if (childData->dirtyGlobal) {
                childData->local = inverse(global) * childData->global;
                childData->dirtyGlobal = false;
                childDirty = true;
            } else if (childData->dirtyLocal || childDirty) {
                childData->global = global * childData->local;
                childData->dirtyLocal = false;
                childDirty = true;
            }
            updateNode(child, childData, childDirty);
        }
    }
}

void Scene2D::disconnectFromParent(uint32_t node) {
    uint32_t parent = nodes[node].parent;
    DynamicArray<uint32_t>& children = nodes[parent].children;
    for (uint32_t i = 0u; i < children.size(); i++) {
        if (children[i] == node) {
            std::memmove(children + i, children + i + 1, children.size() - i - 1);
            children.removeEnd(1u);
            break;
        }
    }
}

void Scene2D::removeNodes(uint32_t node) {
    SceneNode& n = nodes[node];
    entityMap.erase(n.entityID);
    DynamicArray<uint32_t>& children = n.children;
    for (uint32_t i = 0u; i < children.size(); i++) {
        removeNodes(children[i]);
    }
    nodes.remove(node);
}

Scene2D::Scene2D(World* world) : world(world) {
    world->ecs.registerUpdateCallback<Scene2D, &Scene2D::frameUpdate, UpdateOrder::POST_FRAME>(this);
}

uint32_t Scene2D::getRootNode() const {
    return root;
}

uint32_t Scene2D::setRootNode(const Entity& e, const std::string& name) {
    uint32_t id = nodes.emplace(name, e.entityID, UINT32_MAX);
    entityMap[e.entityID] = id;
    root = id;
    return id;
}

uint32_t Scene2D::addNode(uint32_t parent, const Entity& e, const std::string& name) {
    uint32_t id = nodes.emplace(name, e.entityID, parent);
    entityMap[e.entityID] = id;
    nodes[parent].children.add(id);
    return id;
}

void Scene2D::removeNode(uint32_t node) {
    disconnectFromParent(node);
    removeNodes(node);
}

uint32_t Scene2D::getChild(uint32_t node, uint32_t index) {
    return nodes[node].children[index];
}

uint32_t Scene2D::getChildCount(uint32_t node) {
    return nodes[node].children.size();
}

uint32_t Scene2D::getChild(uint32_t node, const char* name, uint32_t nameLength) {
    DynamicArray<uint32_t>& children = nodes[node].children;
    for (uint32_t i = 0u; i < children.size(); i++) {
        uint32_t child = children[i];
        const std::string& childName = nodes[child].name;
        uint32_t maxLength = min(nameLength, static_cast<uint32_t>(childName.length() + 1u));
        for (uint32_t j = 0u; j < maxLength; j++) {
            if (childName[j] != name[j]) {
                goto next;
            }
        }
        return child;
    next:;
    }
    return UINT32_MAX;
}

uint32_t Scene2D::getNode(uint32_t node, const char* path, uint32_t pathLength) {
    uint32_t currentNode = node;
    uint32_t pos = 0u;
    while (path[pos] && pos < pathLength) {
        uint32_t start = pos;
        while (path[pos] && pos < pathLength && path[pos] != '/') {
            pos++;
        }
        uint32_t end = pos;
        currentNode = getChild(currentNode, path + start, end - start);
        if (currentNode == UINT32_MAX) {
            return UINT32_MAX;
        }
        if (path[pos] == '/') {
            pos++;
        }
    }
    return currentNode;
}

std::string& Scene2D::getNodeName(uint32_t node) {
    return nodes[node].name;
}

std::string Scene2D::getNodePath(uint32_t node, uint32_t fromNode) {
    DynamicArray<uint32_t> nodePath;
    uint32_t currentNode = node;
    while (currentNode != fromNode) {
        nodePath.add(currentNode);
        currentNode = getParent(currentNode);
    }
    std::string path;
    for (uint32_t i = nodePath.size(); i >= 1u; i--) {
        uint32_t idx = i - 1u;
        path += getNodeName(nodePath[idx]);
        if (idx > 0u) {
            path += "/";
        }
    }
    return path;
}

uint32_t Scene2D::getParent(uint32_t node) {
    return nodes[node].parent;
}

void Scene2D::setParent(uint32_t node, uint32_t newParent, bool rebase) {
    disconnectFromParent(node);
    nodes[node].parent = newParent;
    nodes[newParent].children.add(node);
    if (rebase) {
        ECS& ecs = world->ecs;
        Transform2D* nodeData = ecs.getComponentPtr<Transform2D>(nodes[node].entityID);
        Transform2D* parentData = ecs.getComponentPtr<Transform2D>(nodes[newParent].entityID);
        if (nodeData && parentData) {
            nodeData->local = inverse(parentData->global) * nodeData->global;
            nodeData->dirtyLocal = true;
        }
    }
}

Entity Scene2D::getEntity(uint32_t node) {
    return Entity{nodes[node].entityID, &world->ecs};
}

uint32_t Scene2D::getNode(uint32_t entityID) {
    return entityMap[entityID];
}

void Scene2D::frameUpdate(double dt) {
    PROFILE_SCOPE(Scene2D_Update)
    if (nodes.size() == 0u) {
        return;
    }
    if (Transform2D* rootData = world->ecs.getComponentPtr<Transform2D>(nodes[root].entityID)) {
        if (rootData->dirtyTRS) {
            rootData->local = trsToMatrix(rootData->position, rootData->rotation, rootData->scale);
            rootData->dirtyLocal = true;
        }
        if (rootData->dirtyGlobal) {
            rootData->local = rootData->global;
        } else if (rootData->dirtyLocal) {
            rootData->global = rootData->local;
        }
        updateNode(root, rootData, rootData->dirtyTRS || rootData->dirtyLocal || rootData->dirtyGlobal);
        rootData->dirtyTRS = false;
        rootData->dirtyLocal = false;
        rootData->dirtyGlobal = false;
    }
}
