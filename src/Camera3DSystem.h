#ifndef CAMERA3DSYSTEM_H_INCLUDED
#define CAMERA3DSYSTEM_H_INCLUDED

#pragma once

#include <iostream>

#include <Camera3D.h>
#include <ecs/ECS.h>

struct Camera3DSystem {
private:
    ECS* ecs;

    void onComponentAdded(Entity e, uint32_t id) {
        Storage<Camera3D>& cameraStorage = ecs->view<Camera3D>();
        Camera3D& cam = cameraStorage.get(id);
        if (!ecs->getComponentID<Transform3D>(e.entityID, cam.transformRef.ID)) {
            std::cerr << "Could not find transform." << std::endl;
        }
    }

    void onComponentRemoved(Entity e, uint32_t id) {
    }

    void update(double dt) {
        Storage<Camera3D>& cameraStorage = ecs->view<Camera3D>();
        DynamicArray<Camera3D>& cameras = cameraStorage.data;
        for (uint32_t i = 0u; i < cameras.size(); i++) {
            Camera3D& cam = cameras[i];
            Transform3D* tx = ecs->getPtr(cam.transformRef);
            cam.cameraPos = prvl::vec3(tx->global[3]);
            cam.inverseCameraTransform = prvl::mat4(prvl::mat3(tx->global));
            cam.cameraTransform = transpose(cam.inverseCameraTransform);
        }
    }

public:
    Camera3DSystem(ECS* ecs) : ecs(ecs) {
        ecs->registerComponentListener<Camera3D, Camera3DSystem, onComponentAdded, onComponentRemoved>(this);
        ecs->registerUpdateCallback<Camera3DSystem, update, UpdateOrder::POST_FRAME>(this);
    }
};

#endif
