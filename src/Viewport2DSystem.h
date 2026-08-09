#ifndef VIEWPORT2DSYSTEM_H_INCLUDED
#define VIEWPORT2DSYSTEM_H_INCLUDED

#pragma once

#include <cstdint>

#include <Camera2D.h>
#include <Viewport2D.h>
#include <ecs/ECS.h>
#include <prvl.h>
#include <scene/2d/Scene2D.h>

struct Viewport2DSystem {
private:
    ECS* ecs;
    Scene2D* scene;
    DynamicArray<uint32_t> viewportEntitiesToLink;

    uint32_t findCamera(uint32_t node, bool checkViewport) {
        Entity e = scene->getEntity(node);
        if (checkViewport) {
            if (e.getComponentPtr<Viewport2D>()) {
                return UINT32_MAX;
            }
        }
        uint32_t id;
        if (ecs->getComponentID<Camera2D>(e.entityID, id)) {
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
        if (Viewport2D* viewport = e.getComponentPtr<Viewport2D>()) {
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
        if (Viewport2D* viewport = e.getComponentPtr<Viewport2D>()) {
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
    Viewport2DSystem(ECS* ecs, Scene2D* scene) : ecs(ecs), scene(scene) {
        ecs->registerComponentListener<Viewport2D, Viewport2DSystem, onComponentAdded, onComponentRemoved>(this);
        ecs->registerUpdateCallback<Viewport2DSystem, update, UpdateOrder::POST_FRAME>(this);
    }
};

#endif
