#include <scene/2d/Scene2D.h>
#include <Transform2D.h>
#include <scene/SceneNode.h>
#include <ecs/ECS.h>

void Scene2D::updateNode(uint32_t node, Transform2D* nodeData, bool dirty) {
    Entity e{nodes[node].entityID, ecs};
    const mat3& global = nodeData->global;
    for(uint32_t i = 0u; i < nodes[node].children.size(); i++) {
        uint32_t child = nodes[node].children[i];
        Entity childEntity{nodes[child].entityID, ecs};
        if(Transform2D* childData = childEntity.getComponentPtr<Transform2D>()) {
            bool childDirty = dirty;
            if(childData->dirtyGlobal) {
                childData->local = inverse(global) * childData->global;
                childData->dirtyGlobal = false;
                childDirty = true;
            } else if(childData->dirtyLocal || childDirty) {
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
    for(uint32_t i = 0u; i < children.size(); i++) {
        if(children[i] == node) {
            std::memmove(children + i, children + i + 1, children.size() - i - 1);
            children.removeEnd(1u);
            break;
        }
    }
}

void Scene2D::removeNodes(uint32_t node) {
    DynamicArray<uint32_t>& children = nodes[node].children;
    for(uint32_t i = 0u; i < children.size(); i++) {
        removeNodes(children[i]);
    }
    nodes.remove(node);
}

Scene2D::Scene2D(ECS* ecs) : ecs(ecs) {
    ecs->registerComponentType<Transform2D>();
    ecs->registerUpdateCallback<Scene2D, frameUpdate, UpdateOrder::FRAME>(this);
}

inline uint32_t Scene2D::getRootNode() const {
    return root;
}

uint32_t Scene2D::setRootNode(Entity e) {
    uint32_t id = nodes.emplace(e.entityID, 0u);
    root = id;
    return id;
}

uint32_t Scene2D::addNode(uint32_t parent, Entity e) {
    uint32_t id = nodes.emplace(e.entityID, parent);
    nodes[parent].children.add(id);
    return id;
}

void Scene2D::removeNode(uint32_t node) {
    disconnectFromParent(node);
    removeNodes(node);
}

Entity Scene2D::getChild(uint32_t node, uint32_t index) {
    return Entity{nodes[nodes[node].children[index]].entityID, ecs};
}

void Scene2D::setParent(uint32_t node, uint32_t newParent, bool rebase) {
    disconnectFromParent(node);
    nodes[node].parent = newParent;
    nodes[newParent].children.add(node);
    if(rebase) {
        Entity e{nodes[node].entityID, ecs};
        Entity parent{nodes[newParent].entityID, ecs};
        Transform2D* nodeData = e.getComponentPtr<Transform2D>();
        Transform2D* parentData = parent.getComponentPtr<Transform2D>();
        if(nodeData && parentData) {
            nodeData->local = inverse(parentData->global) * nodeData->global;
            nodeData->dirtyLocal = true;
        }
    }
}

inline Entity Scene2D::getNode(uint32_t node) {
    return Entity{nodes[node].entityID, ecs};
}

void Scene2D::frameUpdate(double dt) {
    if(nodes.size() == 0u) {
        return;
    }
    Entity r{nodes[root].entityID, ecs};
    if(Transform2D* rootData = r.getComponentPtr<Transform2D>()) {
        if(rootData->dirtyGlobal) {
            rootData->local = rootData->global;
        } else if(rootData->dirtyLocal) {
            rootData->global = rootData->local;
        }
        updateNode(root, rootData, rootData->dirtyLocal || rootData->dirtyGlobal);
        rootData->dirtyLocal = false;
        rootData->dirtyGlobal = false;
    }
}
