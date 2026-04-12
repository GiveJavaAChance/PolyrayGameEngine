#include <scene/3d/SceneNode3D.h>
#include <scene/3d/Scene3D.h>

void Scene3D::updateNode(uint32_t node, Node3D* nodeData, bool dirty) {
    for(uint32_t i = 0u; i < nodes[node].children.size(); i++) {
        uint32_t child = nodes[node].children[i];
        if(Node3D* childData = ecs->getPtr(nodes[child].ref)) {
            bool childDirty = dirty;
            if(childData->dirtyGlobal) {
                childData->local = inverse(nodeData->global) * childData->global;
                childData->dirtyGlobal = false;
                childDirty = true;
            } else if(childData->dirtyLocal || childDirty) {
                childData->global = nodeData->global * childData->local;
                childData->dirtyLocal = false;
                childDirty = true;
            }
            updateNode(child, childData, childDirty);
        }
    }
}

void Scene3D::disconnectFromParent(uint32_t node) {
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

void Scene3D::removeNodes(uint32_t node) {
    DynamicArray<uint32_t>& children = nodes[node].children;
    for(uint32_t i = 0u; i < children.size(); i++) {
        removeNodes(children[i]);
    }
    nodes.remove(node);
}

Scene3D::Scene3D(ECS* ecs) : ecs(ecs) {
    ecs->registerComponentType<Node3D>();
}

inline uint32_t Scene3D::getRootNode() {
    return root;
}

void Scene3D::setRootNode(ComponentRef<Node3D> node) {
    uint32_t id = nodes.emplace(0u, node, 0u);
    nodes[id].id = id;
    root = id;
}

uint32_t Scene3D::addNode(uint32_t parent, ComponentRef<Node3D> node) {
    uint32_t id = nodes.emplace(0u, node, parent);
    nodes[id].id = id;
    nodes[parent].children.add(id);
    return id;
}

void Scene3D::removeNode(uint32_t node) {
    disconnectFromParent(node);
    removeNodes(node);
}

ComponentRef<Node3D> Scene3D::getChild(uint32_t node, uint32_t index) {
    return nodes[nodes[node].children[index]].ref;
}

void Scene3D::setParent(uint32_t node, uint32_t newParent, bool rebase) {
    disconnectFromParent(node);
    nodes[node].parent = newParent;
    nodes[newParent].children.add(node);
    if(rebase) {
        Node3D* nodeData = ecs->getPtr(nodes[node].ref);
        Node3D* parentData = ecs->getPtr(nodes[newParent].ref);
        if(nodeData && parentData) {
            nodeData->local = inverse(parentData->global) * nodeData->global;
            nodeData->dirtyLocal = true;
        }
    }
}

void Scene3D::update() {
    if(Node3D* rootData = ecs->getPtr(nodes[root].ref)) {
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
