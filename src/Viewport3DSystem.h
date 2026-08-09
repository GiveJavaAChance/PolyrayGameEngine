#ifndef VIEWPORT3DSYSTEM_H_INCLUDED
#define VIEWPORT3DSYSTEM_H_INCLUDED

#pragma once

#include <cstdint>

#include <Camera3D.h>
#include <Viewport3D.h>
#include <ecs/ECS.h>
#include <prvl.h>
#include <scene/3d/Scene3D.h>

struct Viewport3DSystem {
private:
    ECS* ecs;
    Scene3D* scene;
    DynamicArray<uint32_t> viewportEntitiesToLink;

    uint32_t findCamera(uint32_t node, bool checkViewport) {
        Entity e = scene->getEntity(node);
        if (checkViewport) {
            if (e.getComponentPtr<Viewport3D>()) {
                return UINT32_MAX;
            }
        }
        uint32_t id;
        if (ecs->getComponentID<Camera3D>(e.entityID, id)) {
            return id;
        }
        uint32_t childCount = scene->getChildCount(node);
        for (uint32_t i = 0u; i < childCount; i++) {
            uint32_t found = findCamera(scene->getChild(node, i), true);
            if (found != UINT32_MAX) {
                return found;
            }
        }
        return UINT32_MAX;
    }

    bool linkEmptyViewport(uint32_t entityID) {
        Entity e{entityID, ecs};
        uint32_t node = scene->getNode(e);
        if (Viewport3D* viewport = e.getComponentPtr<Viewport3D>()) {
            if (viewport->cam.ID == UINT32_MAX) {
                uint32_t camID = findCamera(node, false);
                if (camID != UINT32_MAX) {
                    viewport->cam.ID = camID;
                    return true;
                }
            }
        }
        return false;
    }

    void onComponentAdded(Entity e, uint32_t id) {
        viewportEntitiesToLink.add(e.entityID);
    }

    void onComponentRemoved(Entity e, uint32_t id) {
        if (Viewport3D* viewport = e.getComponentPtr<Viewport3D>()) {
            viewport->fbo.destroy();
        }
    }

    void update(double dt) {
        for (uint32_t i = viewportEntitiesToLink.size(); i >= 1u; i--) {
            if (linkEmptyViewport(viewportEntitiesToLink[i - 1u])) {
                viewportEntitiesToLink[i - 1u] = viewportEntitiesToLink[viewportEntitiesToLink.size() - 1u];
                viewportEntitiesToLink.removeEnd(1u);
            }
        }
    }

public:
    Viewport3DSystem(ECS* ecs, Scene3D* scene) : ecs(ecs), scene(scene) {
        ecs->registerComponentListener<Viewport3D, Viewport3DSystem, onComponentAdded, onComponentRemoved>(this);
        ecs->registerUpdateCallback<Viewport3DSystem, update, UpdateOrder::POST_FRAME>(this);
    }
};

#endif
