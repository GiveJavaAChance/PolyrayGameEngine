#ifndef CAMERA2DSYSTEM_H_INCLUDED
#define CAMERA2DSYSTEM_H_INCLUDED

#pragma once

#include <iostream>

#include <Camera2D.h>
#include <Profiler.h>
#include <ecs/ECS.h>

struct Camera2DSystem {
private:
    ECS* ecs;

    void onComponentAdded(Entity e, uint32_t id) {
        Storage<Camera2D>& cameraStorage = ecs->storage<Camera2D>();
        Camera2D& cam = cameraStorage.get(id);
        if (!ecs->getComponentID<Transform2D>(e.entityID, cam.transformID)) {
            std::cerr << "Could not find transform." << std::endl;
        }
    }

    void onComponentRemoved(Entity e, uint32_t id) {
    }

    void update(double dt) {
        PROFILE_SCOPE(Camera2DSystem_Update)
        Storage<Camera2D>& cameraStorage = ecs->storage<Camera2D>();
        VirtualArray<Camera2D>& cameras = cameraStorage.data;
        for (uint32_t i = 0u; i < cameras.size(); i++) {
            Camera2D& cam = cameras[i];
            Transform2D* tx = ecs->getPtr<Transform2D>(cam.transformID);
            cam.cameraTransform = tx->global;
            cam.inverseCameraTransform = transpose(cam.cameraTransform);
        }
    }

public:
    Camera2DSystem(ECS* ecs) : ecs(ecs) {
        ecs->registerComponentListener<Camera2D, Camera2DSystem, &Camera2DSystem::onComponentAdded, &Camera2DSystem::onComponentRemoved>(this);
        ecs->registerUpdateCallback<Camera2DSystem, &Camera2DSystem::update, UpdateOrder::POST_PHYSICS>(this);
        ecs->registerUpdateCallback<Camera2DSystem, &Camera2DSystem::update, UpdateOrder::POST_FRAME>(this);
    }
};

#endif
