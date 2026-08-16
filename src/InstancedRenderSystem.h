#ifndef INSTANCEDRENDERSYSTEM_H_INCLUDED
#define INSTANCEDRENDERSYSTEM_H_INCLUDED

#pragma once

#include <cstring>

#include <Allocator.h>
#include <RenderObject.h>
#include <World.h>
#include <serial/Serial.h>
#include <structure/UnorderedRegistry.h>

struct RenderInstance {
    uint32_t objectID;
    uint32_t componentRef;

    explicit RenderInstance(uint32_t objID) noexcept : objectID(objID), componentRef(UINT32_MAX) {
    }

    RenderInstance() noexcept : RenderInstance(UINT32_MAX) {
    }
};

template <>
struct Serial<RenderInstance> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        RenderInstance* r = world->ecs.getPtr<RenderInstance>(componentID);
        output.write(r->objectID);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        RenderInstance r;
        input.read(r.objectID);
        e.addComponent(r);
    }
};

template <>
struct ExportInfo<RenderInstance> {
    constexpr static Export __export__[] = {
        {offsetof(RenderInstance, objectID), EXPORT_UINT, "Object ID"},
    };
};

template <typename T>
void defaultExtract(const T& in, T* out) {
    *out = in;
}

template <typename T, typename U = T, void (*Extract)(const T&, U*) = defaultExtract>
struct InstancedRenderSystem {
private:
    ECS* ecs;
    UnorderedRegistry<RenderObject*> objects;

    DynamicArray<U> buffer;

    void onComponentAdded(Entity e, uint32_t id) {
        Storage<RenderInstance>& instanceStorage = ecs->view<RenderInstance>();
        RenderInstance& i = instanceStorage.get(id);
        if (!ecs->getComponentID<T>(e.entityID, i.componentRef)) {
            std::cerr << "Could not find instance." << std::endl;
        }
    }

    void onComponentRemoved(Entity e, uint32_t id) {
    }

public:
    InstancedRenderSystem(ECS* ecs) : ecs(ecs) {
        ecs->registerComponentListener<RenderInstance, InstancedRenderSystem<T, U, Extract>, onComponentAdded, onComponentRemoved>(this);
    }

    uint32_t addObject(RenderObject* obj) {
        return objects.add(obj);
    }

    void render() {
        Storage<RenderInstance>& instanceStorage = ecs->view<RenderInstance>();
        DynamicArray<RenderInstance>& instances = instanceStorage.data;
        if(objects.size() == 0u || instances.size() == 0u) {
            return;
        }
        uint32_t* instanceCount = alloc<uint32_t>(objects.size());
        std::memset(instanceCount, 0, objects.size() * sizeof(uint32_t));
        for (uint32_t i = 0u; i < instances.size(); i++) {
            uint32_t objectID = instances[i].objectID;
            if (objectID == UINT32_MAX) {
                continue;
            }
            instanceCount[objects.reg[objectID]]++;
        }
        uint32_t maxCount = 0u;
        for (uint32_t i = 0u; i < objects.size(); i++) {
            if (instanceCount[i] > maxCount) {
                maxCount = instanceCount[i];
            }
        }
        free(instanceCount);
        buffer.ensureCapacity(maxCount);
        for (uint32_t i = 0u; i < objects.size(); i++) {
            uint32_t count = 0u;
            for (uint32_t j = 0u; j < instances.size(); j++) {
                RenderInstance& in = instances[j];
                if (in.objectID == UINT32_MAX) {
                    continue;
                }
                if (objects.reg[in.objectID] == i) {
                    T data = ecs->read<T>(in.componentRef);
                    Extract(data, buffer.data() + count);
                    ++count;
                }
            }
            objects.arr[i]->uploadInstances(buffer.data(), count);
        }
        for (uint32_t i = 0u; i < objects.size(); i++) {
            const RenderObject& obj = *objects.data()[i];
            if (obj.vertexCount == 0u || obj.instanceCount == 0u) {
                continue;
            }
            obj.shader.use();
            if (obj.texture) {
                glBindTextureUnit(0, obj.texture->ID);
            }
            glBindVertexArray(obj.vao);
            glDrawArraysInstanced(obj.mode, 0, obj.vertexCount, obj.instanceCount);
        }
    }
};

#endif
