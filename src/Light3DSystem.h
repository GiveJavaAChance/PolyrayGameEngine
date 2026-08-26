#ifndef LIGHT3DSYSTEM_H_INCLUDED
#define LIGHT3DSYSTEM_H_INCLUDED

#pragma once

#include <ShadowSystem.h>
#include <Transform3D.h>
#include <World.h>
#include <prvl.h>

struct DirectionalLight3D {
    vec3 color;
    float strength;
    bool castShadow;
    float shadowWidth;
    float shadowDepth;
    uint32_t shadowId;
    uint32_t transformID = UINT32_MAX;

    DirectionalLight3D() {
    }

    DirectionalLight3D(const vec3& color, float strength, bool castShadow = true, float shadowWidth = 100.0f, float shadowDepth = 100.0f)
        : color(color), strength(strength), castShadow(castShadow), shadowWidth(shadowWidth), shadowDepth(shadowDepth) {
    }
};

struct SpotLight3D {
    vec3 color;
    float strength;
    float distanceAttenuation;
    float spotAngle;
    bool castShadow;
    float shadowNearZ;
    uint32_t shadowId;
    uint32_t transformID = UINT32_MAX;

    SpotLight3D() {
    }

    SpotLight3D(const vec3& color, float strength, float distanceAttenuation, float spotAngle, bool castShadow = true, float shadowNearZ = 0.1f)
        : color(color), strength(strength), distanceAttenuation(distanceAttenuation), spotAngle(spotAngle), castShadow(castShadow), shadowNearZ(shadowNearZ) {
    }
};

struct PointLight3D {
    vec3 color;
    float strength;
    float distanceAttenuation;
    bool castShadow;
    float shadowNearZ;
    uint32_t shadowId[6u];
    uint32_t transformID = UINT32_MAX;

    PointLight3D() {
    }

    PointLight3D(const vec3& color, float strength, float distanceAttenuation, bool castShadow = true, float shadowNearZ = 0.1f)
        : color(color), strength(strength), distanceAttenuation(distanceAttenuation), castShadow(castShadow), shadowNearZ(shadowNearZ) {
    }
};

template <>
struct Serial<DirectionalLight3D> {
    constexpr static uint32_t SIZE = offsetof(DirectionalLight3D, shadowId);

    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        DirectionalLight3D* light = world->ecs.getPtr<DirectionalLight3D>(componentID);
        output.write(light, SIZE);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        DirectionalLight3D light{};
        input.read(&light, SIZE);
        e.addComponent(light);
    }
};

template <>
struct Serial<SpotLight3D> {
    constexpr static uint32_t SIZE = offsetof(SpotLight3D, shadowId);

    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        SpotLight3D* light = world->ecs.getPtr<SpotLight3D>(componentID);
        output.write(light, SIZE);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        SpotLight3D light{};
        input.read(&light, SIZE);
        e.addComponent(light);
    }
};

template <>
struct Serial<PointLight3D> {
    constexpr static uint32_t SIZE = offsetof(PointLight3D, shadowId);

    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        PointLight3D* light = world->ecs.getPtr<PointLight3D>(componentID);
        output.write(light, SIZE);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        PointLight3D light{};
        input.read(&light, SIZE);
        e.addComponent(light);
    }
};

struct Light3DSystem {
private:
    constexpr static mat3 CUBE_TRANSFORMS[6u]{
        prvl::mat3({0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}),
        prvl::mat3({0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}),
        prvl::mat3({1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}),
        prvl::mat3({1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}),
        prvl::mat3({-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}),
        prvl::mat3({1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}),
    };

    struct GpuDirectionalLight3D {
        vec3 color;
        float strength;
        vec3 dir;
        uint32_t shadowIdx;
    };

    struct GpuSpotLight3D {
        vec3 color;
        float strength;
        vec3 pos;
        float distanceAttenuation;
        vec3 dir;
        float spotCosAngle;
        uint32_t shadowIdx;
        uint32_t __padding__[3u];
    };

    struct GpuPointLight3D {
        vec3 color;
        float strength;
        vec3 pos;
        float distanceAttenuation;
        uint32_t shadowIdx[6u];
        uint32_t __padding__[2u];
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

        ecs->registerComponentListener<DirectionalLight3D, Light3DSystem, onDirectionalLightAdded, onDirectionalLightRemoved>(this);
        ecs->registerComponentListener<SpotLight3D, Light3DSystem, onSpotLightAdded, onSpotLightRemoved>(this);
        ecs->registerComponentListener<PointLight3D, Light3DSystem, onPointLightAdded, onPointLightRemoved>(this);
        ecs->registerUpdateCallback<Light3DSystem, update, UpdateOrder::POST_FRAME>(this);
    }

    void update(double dt) {
        Storage<Transform3D>& transforms = ecs->view<Transform3D>();

        DynamicArray<DirectionalLight3D>& directionalLights = ecs->view<DirectionalLight3D>().data;
        uint32_t dirBufferSize = directionalLights.size() * sizeof(GpuDirectionalLight3D) + 16u;
        uint8_t* dirBuffer = alloc<uint8_t>(dirBufferSize);
        uint32_t directionalLightCount = directionalLights.size();
        std::memcpy(dirBuffer, &directionalLightCount, 4u);

        for (uint32_t i = 0u; i < directionalLights.size(); i++) {
            DirectionalLight3D& light = directionalLights[i];
            mat4 tx = transforms.get(light.transformID).global;
            if (light.castShadow) {
                shadowSystem->updateShadowCamera(light.shadowId, tx);
            }
            GpuDirectionalLight3D dir{light.color, light.strength, prvl::vec3(tx[2]), light.castShadow ? shadowSystem->shadowCameras.reg[light.shadowId] : UINT32_MAX};
            std::memcpy(dirBuffer + i * sizeof(GpuDirectionalLight3D) + 16u, &dir, sizeof(GpuDirectionalLight3D));
        }

        DynamicArray<SpotLight3D>& spotLights = ecs->view<SpotLight3D>().data;
        uint32_t spotBufferSize = spotLights.size() * sizeof(GpuSpotLight3D) + 16u;
        uint8_t* spotBuffer = alloc<uint8_t>(spotBufferSize);
        uint32_t spotLightCount = spotLights.size();
        std::memcpy(spotBuffer, &spotLightCount, 4u);

        for (uint32_t i = 0u; i < spotLights.size(); i++) {
            SpotLight3D& light = spotLights[i];
            mat4 tx = transforms.get(light.transformID).global;
            if (light.shadowId != UINT32_MAX) {
                shadowSystem->updateShadowCamera(light.shadowId, tx);
            }
            GpuSpotLight3D spot{light.color, light.strength, prvl::vec3(tx[3]), light.distanceAttenuation, prvl::vec3(tx[2]), cosf(light.spotAngle * 0.5f), light.castShadow ? shadowSystem->shadowCameras.reg[light.shadowId] : UINT32_MAX};
            std::memcpy(spotBuffer + i * sizeof(GpuSpotLight3D) + 16u, &spot, sizeof(GpuSpotLight3D));
        }

        DynamicArray<PointLight3D>& pointLights = ecs->view<PointLight3D>().data;
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
                    mat3 r = prvl::mat3(tx) * CUBE_TRANSFORMS[j];
                    mat4 tr = prvl::mat4(r);
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
