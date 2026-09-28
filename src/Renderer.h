#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include "ecs/ECS.h"
#include <unordered_map>
#pragma once

#include <BindingRegistry.h>
#include <MaterialType.h>
#include <Profiler.h>
#include <RenderGroupInfo.h>
#include <RenderInstance.h>
#include <RenderObject.h>
#include <RenderState.h>
#include <ecs/ComponentMetadata.h>
#include <structure/DynamicArray.h>
#include <structure/UnorderedRegistry.h>

struct Renderer {
private:
    struct Render {
        RenderObject object;
        uint32_t materialID;
        uint32_t instanceBufferCapacity;
        DynamicArray<uint32_t> instances;
    };

    struct RenderGroup {
        RenderGroupInfo info;

        ShaderBuffer materialInstanceBuffer;
        void* materialInstanceData;
        DynamicArray<RenderState> materialRenderStates;
        Registry materialInstanceRegistry;

        UnorderedRegistry<Render> objects;
    };

    ECS* ecs;

    UnorderedRegistry<RenderGroup> groups;
    std::unordered_map<GLuint, uint32_t> groupMap;

    void onComponentAdded(Entity e, uint32_t id) {
        Storage<RenderInstance>& instanceStorage = ecs->view<RenderInstance>();
        RenderInstance& i = instanceStorage.get(id);
        RenderGroup& group = groups[i.objectID >> 16u];
        group.objects[i.objectID & 0xFFFFu].instances.add(id);
        if (!ecs->reflectGetComponentID(group.info.componentType, e.entityID, i.componentID)) {
            std::cerr << "Could not find instance." << std::endl;
        }
    }

    void onComponentRemoved(Entity e, uint32_t id) {
    }

    DynamicArray<uint8_t> uploadBuffer;

public:
    uint32_t materialInstanceBinding;

    Renderer(ECS* ecs) : ecs(ecs), materialInstanceBinding(BindingRegistry::allocateBufferBinding()) {
        ecs->registerComponentListener<RenderInstance, Renderer, &Renderer::onComponentAdded, &Renderer::onComponentRemoved>(this);
        ecs->registerUpdateCallback<Renderer, &Renderer::update, UpdateOrder::POST_FRAME>(this);
        ShaderManager::setValue("MAT_INS_IDX", materialInstanceBinding);
    }

    uint32_t getOrCreateGroup(const RenderGroupInfo& groupInfo) {
        GLuint programID = groupInfo.materialType.forward.ID;
        std::unordered_map<GLuint, uint32_t>::iterator it = groupMap.find(programID);
        if (it == groupMap.end()) {
            uint32_t groupID = groups.emplace(groupInfo, GL_STATIC_DRAW, nullptr);
            groupMap[programID] = groupID;
            return groupID;
        }
        return it->second;
    }

    template <typename T>
    uint32_t addMaterialInstance(uint32_t group, const T& instance, const RenderState& renderState) {
        RenderGroup& g = groups[group];
        if (!g.materialInstanceData) {
            g.materialInstanceData = new DynamicArray<T>();
        }
        DynamicArray<T>* instanceData = reinterpret_cast<DynamicArray<T>*>(g.materialInstanceData);
        instanceData->add(instance);
        g.materialRenderStates.add(renderState);
        uint32_t id = g.materialInstanceRegistry.create();
        g.materialInstanceBuffer.uploadData(instanceData->data(), instanceData->size());
        return id;
    }

    uint32_t createObject(uint32_t group, uint32_t materialID) {
        RenderGroup& g = groups[group];
        return group << 16u | g.objects.emplace(g.info.materialType.vertexLayout, materialID, 0u);
    }

    RenderObject& getObject(uint32_t objectID) {
        return groups[objectID >> 16u].objects[objectID & 0xFFFFu].object;
    }

    void update(double dt) {
        PROFILE_SCOPE(Renderer_Update)
        if (groups.size() == 0u) {
            return;
        }
        Storage<RenderInstance>& instanceStorage = ecs->view<RenderInstance>();
        if (instanceStorage.data.size() == 0u) {
            return;
        }
        for (uint32_t i = 0u; i < groups.size(); i++) {
            RenderGroup& group = groups.arr[i];
            if (group.objects.size() == 0u) {
                continue;
            }
            const RenderGroupInfo& info = group.info;
            const ComponentMetadata& meta = ComponentRegistry::metadata[info.componentType];
            void* buffer = _mm_malloc(meta.size, meta.alignment);
            for (uint32_t j = 0u; j < group.objects.size(); j++) {
                Render& object = group.objects.arr[j];
                uint32_t size = object.instances.size() * (info.memberSize + 4u);
                if (size == 0u) {
                    continue;
                }
                uploadBuffer.ensureCapacity(size);
                uint32_t materialInstanceIdx = group.materialInstanceRegistry[object.materialID];
                uint8_t* data = uploadBuffer.data();
                for (uint32_t k = 0u; k < object.instances.size(); k++) {
                    uint32_t instanceID = object.instances[k];
                    RenderInstance& instance = instanceStorage.get(instanceID);
                    if (instance.componentID != UINT32_MAX) {
                        void* src = ecs->reflectGetPtr(info.componentType, instance.componentID);
                        if (src == nullptr) {
                            src = buffer;
                            ecs->reflectRead(info.componentType, instance.componentID, src);
                        }
                        uint8_t* dst = data + k * (info.memberSize + 4u);
                        std::memcpy(dst, &materialInstanceIdx, 4u);
                        std::memcpy(dst + 4u, reinterpret_cast<uint8_t*>(src) + info.memberOffset, info.memberSize);
                    }
                }
                if (size > object.instanceBufferCapacity) {
                    object.instanceBufferCapacity = (size * 3u) >> 1u;
                    object.object.instanceVbo.setSize(object.instanceBufferCapacity);
                }
                object.object.instanceVbo.uploadPartialData(data, size, 0u);
                object.object.instanceCount = object.instances.size();
            }
            _mm_free(buffer);
        }
    }

    void render(RenderMode mode) {
        for (uint32_t i = 0u; i < groups.size(); i++) {
            RenderGroup& group = groups.arr[i];
            group.info.materialType.use(mode);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, materialInstanceBinding, group.materialInstanceBuffer.ID);
            for (uint32_t i = 0u; i < group.objects.size(); i++) {
                Render& obj = group.objects.arr[i];
                RenderState& renderState = group.materialRenderStates[group.materialInstanceRegistry[obj.materialID]];
                if (renderState.doubleSided) {
                    glDisable(GL_CULL_FACE);
                } else {
                    glEnable(GL_CULL_FACE);
                }
                obj.object.render();
            }
        }
    }
};

#endif
