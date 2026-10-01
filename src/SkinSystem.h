#ifndef SKINSYSTEM_H_INCLUDED
#define SKINSYSTEM_H_INCLUDED

#include "utils/prvl_assert.h"
#pragma once

#include <BindingRegistry.h>
#include <Profiler.h>
#include <SkinInstance.h>
#include <Transform3D.h>
#include <ecs/ECS.h>
#include <glad/glad.h>
#include <prvl.h>
#include <rendering/DynamicShaderBuffer.h>
#include <shader/ShaderManager.h>
#include <structure/DynamicArray.h>
#include <structure/UnorderedRegistry.h>

#include <utils/prvl_assert.h>

struct Skin {
    uint32_t jointIdx;
    DynamicArray<uint32_t> transformIDs;
    DynamicArray<mat4> inverseBindMatrices;
};

struct SkinSystem {
private:
    ECS* ecs;
    UnorderedRegistry<Skin> skins;

    uint32_t skinJointCount = 0u;
    DynamicArray<mat4> skinJointUploadBuffer;

    DynamicShaderBuffer skinJointBuffer;
    DynamicShaderBuffer skinInstanceBuffer;

    void onComponentAdded(Entity e, uint32_t id) {
        SkinInstance* instance = ecs->getPtr<SkinInstance>(id);
        PRVL_ASSERT(ecs->getComponentID<Transform3D>(e.entityID, instance->transformID), "Failed to get transform")
    }

    void onComponentRemoved(Entity e, uint32_t id) {
    }

public:
    GLuint skinJointBinding;
    GLuint skinInstanceBinding;

    SkinSystem(ECS* ecs)
        : ecs(ecs), skinJointBuffer(1ull << 20u, GL_DYNAMIC_STORAGE_BIT),
          skinInstanceBuffer(1ull << 16u, GL_DYNAMIC_STORAGE_BIT),
          skinJointBinding(BindingRegistry::bindBufferBase(skinJointBuffer.buffer, GL_SHADER_STORAGE_BUFFER)),
          skinInstanceBinding(BindingRegistry::bindBufferBase(skinInstanceBuffer.buffer, GL_SHADER_STORAGE_BUFFER)) {
        ecs->registerComponentListener<SkinInstance, SkinSystem, &SkinSystem::onComponentAdded, &SkinSystem::onComponentRemoved>(this);
        ecs->registerUpdateCallback<SkinSystem, &SkinSystem::update, UpdateOrder::POST_FRAME>(this);
        ShaderManager::setValue("SKIN_JOINT_IDX", skinJointBinding);
        ShaderManager::setValue("SKIN_INS_IDX", skinInstanceBinding);
    }

    inline uint32_t createSkin(uint32_t jointCount) {
        uint32_t off = skinJointCount;
        skinJointCount += jointCount;
        skinInstanceBuffer.add(&off, sizeof(uint32_t));
        return skins.emplace(off, jointCount);
    }

    inline Skin& getSkin(uint32_t skinID) {
        return skins[skinID];
    }

    void update(double dt) {
        PROFILE_SCOPE(SkinSystem_Update)
        if (skinInstanceBuffer.resized) {
            skinInstanceBuffer.resized = false;
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, skinInstanceBinding, skinInstanceBuffer.buffer.ID);
        }
        VirtualArray<SkinInstance>& skinInstances = ecs->view<SkinInstance>().data;
        if (skinInstances.size() == 0u) {
            return;
        }
        Storage<Transform3D>& transforms = ecs->view<Transform3D>();
        for (uint32_t i = 0u; i < skinInstances.size(); i++) {
            SkinInstance& instance = skinInstances[i];
            instance.data.skinIdx = skins.reg[instance.skinID];
            instance.data.tx = transforms.get(instance.transformID).global;
        }
        skinJointUploadBuffer.ensureCapacity(skinJointCount);
        for (uint32_t i = 0u; i < skins.size(); i++) {
            Skin& skin = skins.arr[i];
            for (uint32_t j = 0u; j < skin.transformIDs.size(); j++) {
                skinJointUploadBuffer[skin.jointIdx + j] = transforms.get(skin.transformIDs[j]).global * skin.inverseBindMatrices[j];
            }
        }
        skinJointBuffer.pos = 0ull;
        skinJointBuffer.add(skinJointUploadBuffer.data(), skinJointCount * sizeof(mat4));
        if (skinJointBuffer.resized) {
            skinJointBuffer.resized = false;
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, skinJointBinding, skinJointBuffer.buffer.ID);
        }
    }
};

#endif