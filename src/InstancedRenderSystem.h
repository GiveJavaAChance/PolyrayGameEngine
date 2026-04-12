#ifndef INSTANCEDRENDERSYSTEM_H_INCLUDED
#define INSTANCEDRENDERSYSTEM_H_INCLUDED

#pragma once

#include <cstring>

#include <ecs/ECS.h>
#include <structure/UnorderedRegistry.h>
#include <Allocator.h>
#include <RenderObject.h>

template<typename T>
struct RenderInstance {
    uint32_t objectID;
    ComponentRef<T> ref;

    explicit RenderInstance(uint32_t objID) noexcept : objectID(objID), ref{0u} {
    }
};

template<typename T>
struct InstancedRenderSystem {
private:
    ECS* ecs;
    UnorderedRegistry<RenderObject*> objects;

    void onComponentAdded(Entity e, uint32_t id) {
        Storage<RenderInstance<T>>& instanceStorage = ecs->view<RenderInstance<T>>();
        RenderInstance<T>& i = instanceStorage.get(id);
        if(!ecs->getComponentID<T>(e, i.ref.ID)) {
            std::cerr << "Could not find instance." << std::endl;
        }
    }

    void onComponentRemoved(Entity e, uint32_t id) {
    }

    void update() {
        Storage<RenderInstance<T>>& instanceStorage = ecs->view<RenderInstance<T>>();
        DynamicArray<RenderInstance<T>>& instances = instanceStorage.data;
        uint32_t* instanceCount = alloc<uint32_t>(objects.size());
        std::memset(instanceCount, 0, objects.size() * sizeof(uint32_t));
        for(uint32_t i = 0u; i < instances.size(); i++) {
            instanceCount[objects.reg[instances[i].objectID]]++;
        }
        uint32_t maxCount = 0u;
        for(uint32_t i = 0u; i < objects.size(); i++) {
            if(instanceCount[i] > maxCount) {
                maxCount = instanceCount[i];
            }
        }
        free(instanceCount);
        T* buffer = alloc<T>(maxCount);
        for(uint32_t i = 0u; i < objects.size(); i++) {
            uint32_t count = 0u;
            for(uint32_t j = 0u; j < instances.size(); j++) {
                if(objects.reg[instances[j].objectID] == i) {
                    buffer[count++] = ecs->read(instances[j].ref);
                }
            }
            objects.arr[i]->uploadInstances(buffer, count);
        }
        free(buffer);
    }

public:
    InstancedRenderSystem(ECS* ecs) : ecs(ecs) {
        ecs->registerComponentType<RenderInstance<T>>();
        ecs->registerComponentListener<RenderInstance<T>, InstancedRenderSystem<T>, onComponentAdded, onComponentRemoved>(this);
        ecs->registerUpdateCallback<InstancedRenderSystem<T>, update, UpdateOrder::PRE_RENDER>(this);
    }

    uint32_t addObject(RenderObject* obj) {
        return objects.add(obj);
    }
};

#endif
