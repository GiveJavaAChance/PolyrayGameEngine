#ifndef LIGHT3DSYSTEM_H_INCLUDED
#define LIGHT3DSYSTEM_H_INCLUDED

#pragma once

#include <Profiler.h>
#include <ShadowSystem.h>
#include <Transform3D.h>
#include <World.h>
#include <prvl.h>

#include <light/3d/DirectionalLight3D.h>
#include <light/3d/PointLight3D.h>
#include <light/3d/SpotLight3D.h>

#include <gpu_types/GpuDirectionalLight3D.h>
#include <gpu_types/GpuPointLight3D.h>
#include <gpu_types/GpuSpotLight3D.h>

struct Light3DSystem {
private:
    constexpr static mat3 CUBE_TRANSFORMS[6u]{
        prvl::mat3({0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}),
        prvl::mat3({0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}),
        prvl::mat3({1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}),
        prvl::mat3({1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}),
        prvl::mat3({1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}),
        prvl::mat3({-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}),
    };

    ECS* ecs;
    ShadowSystem* shadowSystem;

    ShaderBuffer directionalLightBuffer;
    ShaderBuffer spotLightBuffer;
    ShaderBuffer pointLightBuffer;

    void onDirectionalLightAdded(Entity e, uint32_t id) {
        DirectionalLight3D* light = ecs->getPtr<DirectionalLight3D>(id);
        if (light->castShadow) {
            light->shadowId = shadowSystem->createOrthographicShadowCaster(light->shadowWidth, light->shadowDepth);
        }
        if (!ecs->getComponentID<Transform3D>(e.entityID, light->transformID)) {
            std::cerr << "Could not find transform." << std::endl;
        }
    }

    void onDirectionalLightRemoved(Entity e, uint32_t id) {
        DirectionalLight3D* light = ecs->getPtr<DirectionalLight3D>(id);
        if (light->castShadow) {
            shadowSystem->removeShadowCaster(light->shadowId);
        }
    }

    void onSpotLightAdded(Entity e, uint32_t id) {
        SpotLight3D* light = ecs->getPtr<SpotLight3D>(id);
        if (light->castShadow) {
            light->shadowId = shadowSystem->createPerspectiveShadowCaster(light->spotAngle, light->shadowNearZ);
        }
        if (!ecs->getComponentID<Transform3D>(e.entityID, light->transformID)) {
            std::cerr << "Could not find transform." << std::endl;
        }
    }

    void onSpotLightRemoved(Entity e, uint32_t id) {
        SpotLight3D* light = ecs->getPtr<SpotLight3D>(id);
        if (light->castShadow) {
            shadowSystem->removeShadowCaster(light->shadowId);
        }
    }

    void onPointLightAdded(Entity e, uint32_t id) {
        PointLight3D* light = ecs->getPtr<PointLight3D>(id);
        if (light->castShadow) {
            for (uint32_t i = 0u; i < 6u; i++) {
                light->shadowId[i] = shadowSystem->createPerspectiveShadowCaster(1.57079632679f, light->shadowNearZ);
            }
        }
        if (!ecs->getComponentID<Transform3D>(e.entityID, light->transformID)) {
            std::cerr << "Could not find transform." << std::endl;
        }
    }

    void onPointLightRemoved(Entity e, uint32_t id) {
        PointLight3D* light = ecs->getPtr<PointLight3D>(id);
        if (light->castShadow) {
            for (uint32_t i = 0u; i < 6u; i++) {
                shadowSystem->removeShadowCaster(light->shadowId[i]);
            }
        }
    }

public:
    Light3DSystem(ECS* ecs, ShadowSystem* shadowSystem, uint32_t maxDirectionalLights, uint32_t maxSpotLights, uint32_t maxPointLights)
        : ecs(ecs), shadowSystem(shadowSystem), directionalLightBuffer(GL_DYNAMIC_DRAW), spotLightBuffer(GL_DYNAMIC_DRAW), pointLightBuffer(GL_DYNAMIC_DRAW) {
        directionalLightBuffer.setSize(sizeof(GpuDirectionalLight3D) * maxDirectionalLights + 16u);
        spotLightBuffer.setSize(sizeof(GpuSpotLight3D) * maxSpotLights + 16u);
        pointLightBuffer.setSize(sizeof(GpuPointLight3D) * maxPointLights + 16u);
        ShaderManager::setValue("DIRECTIONAL_LIGHT3D_IDX", BindingRegistry::bindBufferBase(directionalLightBuffer, GL_SHADER_STORAGE_BUFFER));
        ShaderManager::setValue("SPOT_LIGHT3D_IDX", BindingRegistry::bindBufferBase(spotLightBuffer, GL_SHADER_STORAGE_BUFFER));
        ShaderManager::setValue("POINT_LIGHT3D_IDX", BindingRegistry::bindBufferBase(pointLightBuffer, GL_SHADER_STORAGE_BUFFER));

        ecs->registerComponentListener<DirectionalLight3D, Light3DSystem, &Light3DSystem::onDirectionalLightAdded, &Light3DSystem::onDirectionalLightRemoved>(this);
        ecs->registerComponentListener<SpotLight3D, Light3DSystem, &Light3DSystem::onSpotLightAdded, &Light3DSystem::onSpotLightRemoved>(this);
        ecs->registerComponentListener<PointLight3D, Light3DSystem, &Light3DSystem::onPointLightAdded, &Light3DSystem::onPointLightRemoved>(this);
        ecs->registerUpdateCallback<Light3DSystem, &Light3DSystem::update, UpdateOrder::POST_FRAME>(this);
    }

    void update(double dt) {
        PROFILE_SCOPE(Light3DSystem_Update)
        Storage<Transform3D>& transforms = ecs->view<Transform3D>();

        VirtualArray<DirectionalLight3D>& directionalLights = ecs->view<DirectionalLight3D>().data;
        uint32_t dirBufferSize = directionalLights.size() * sizeof(GpuDirectionalLight3D) + 16u;
        uint8_t* dirBuffer = alloc<uint8_t>(dirBufferSize);
        uint32_t directionalLightCount = directionalLights.size();
        std::memcpy(dirBuffer, &directionalLightCount, 4u);

        for (uint32_t i = 0u; i < directionalLights.size(); i++) {
            DirectionalLight3D& light = directionalLights[i];
            mat4 tx = transforms.get(light.transformID).global;
            tx[0] = prvl::vec4(normalize(prvl::vec3(tx[0])), 0.0f);
            tx[1] = prvl::vec4(normalize(prvl::vec3(tx[1])), 0.0f);
            tx[2] = prvl::vec4(normalize(prvl::vec3(tx[2])), 0.0f);
            if (light.castShadow) {
                shadowSystem->updateShadowCamera(light.shadowId, tx);
            }
            GpuDirectionalLight3D dir{light.color, light.strength, prvl::vec3(tx[2]), light.castShadow ? shadowSystem->shadowCameras.reg[light.shadowId] : UINT32_MAX};
            std::memcpy(dirBuffer + i * sizeof(GpuDirectionalLight3D) + 16u, &dir, sizeof(GpuDirectionalLight3D));
        }

        VirtualArray<SpotLight3D>& spotLights = ecs->view<SpotLight3D>().data;
        uint32_t spotBufferSize = spotLights.size() * sizeof(GpuSpotLight3D) + 16u;
        uint8_t* spotBuffer = alloc<uint8_t>(spotBufferSize);
        uint32_t spotLightCount = spotLights.size();
        std::memcpy(spotBuffer, &spotLightCount, 4u);

        for (uint32_t i = 0u; i < spotLights.size(); i++) {
            SpotLight3D& light = spotLights[i];
            mat4 tx = transforms.get(light.transformID).global;
            tx[0] = prvl::vec4(normalize(prvl::vec3(tx[0])), 0.0f);
            tx[1] = prvl::vec4(normalize(prvl::vec3(tx[1])), 0.0f);
            tx[2] = prvl::vec4(normalize(prvl::vec3(tx[2])), 0.0f);
            if (light.shadowId != UINT32_MAX) {
                shadowSystem->updateShadowCamera(light.shadowId, tx);
            }
            GpuSpotLight3D spot{light.color, light.strength, prvl::vec3(tx[3]), light.distanceAttenuation, prvl::vec3(tx[2]), cos(light.spotAngle * 0.5f), light.castShadow ? shadowSystem->shadowCameras.reg[light.shadowId] : UINT32_MAX};
            std::memcpy(spotBuffer + i * sizeof(GpuSpotLight3D) + 16u, &spot, sizeof(GpuSpotLight3D));
        }

        VirtualArray<PointLight3D>& pointLights = ecs->view<PointLight3D>().data;
        uint32_t pointBufferSize = pointLights.size() * sizeof(GpuPointLight3D) + 16u;
        uint8_t* pointBuffer = alloc<uint8_t>(pointBufferSize);
        uint32_t pointLightCount = pointLights.size();
        std::memcpy(pointBuffer, &pointLightCount, 4u);

        for (uint32_t i = 0u; i < pointLights.size(); i++) {
            PointLight3D& light = pointLights[i];
            mat4 tx = transforms.get(light.transformID).global;
            vec4 t = tx[3];
            GpuPointLight3D point{light.color, light.strength, prvl::vec3(t), light.distanceAttenuation};
            if (light.castShadow) {
                for (uint32_t j = 0u; j < 6u; j++) {
                    mat4 tr = prvl::mat4(CUBE_TRANSFORMS[j]);
                    tr[3] = t;
                    shadowSystem->updateShadowCamera(light.shadowId[j], tr);
                    point.shadowIdx[j] = shadowSystem->shadowCameras.reg[light.shadowId[j]];
                }
            } else {
                point.shadowIdx[0u] = UINT32_MAX;
            }
            std::memcpy(pointBuffer + i * sizeof(GpuPointLight3D) + 16u, &point, sizeof(GpuPointLight3D));
        }

        directionalLightBuffer.uploadPartialData(dirBuffer, dirBufferSize, 0u);
        spotLightBuffer.uploadPartialData(spotBuffer, spotBufferSize, 0u);
        pointLightBuffer.uploadPartialData(pointBuffer, pointBufferSize, 0u);

        free(dirBuffer);
        free(spotBuffer);
        free(pointBuffer);
    }
};

#endif
