#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#pragma once

#include <Material.h>
#include <RenderObject.h>
#include <structure/DynamicArray.h>
#include <structure/UnorderedRegistry.h>

struct RenderInstance {
    uint32_t objectID;

    uint32_t componentID = UINT32_MAX;

    RenderInstance(uint32_t objectID = UINT32_MAX) : objectID(objectID) {
    }
};

template <>
struct Serial<RenderInstance> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        RenderInstance* r = world->ecs.getPtr<RenderInstance>(componentID);
        output.write(r->objectID);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        RenderInstance r{0u};
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

struct Renderer {
private:
    struct Render {
        RenderObject object;
        DynamicArray<uint32_t> instances;
    };

    struct RenderGroup {
        Material* material;
        VertexLayoutInfo layout;
        uint32_t componentType;
        void (*extract)(const void*, void*);
        uint32_t dataSize;
        UnorderedRegistry<Render> objects;
    };

    ECS* ecs;

    UnorderedRegistry<RenderGroup> groups;

    void onComponentAdded(Entity e, uint32_t id) {
        Storage<RenderInstance>& instanceStorage = ecs->view<RenderInstance>();
        RenderInstance& i = instanceStorage.get(id);
        RenderGroup& group = groups[i.objectID >> 16u];
        group.objects[i.objectID & 0xFFFFu].instances.add(id);
        if (!ecs->reflectGetComponentID(group.componentType, e.entityID, i.componentID)) {
            std::cerr << "Could not find instance." << std::endl;
        }
    }

    void onComponentRemoved(Entity e, uint32_t id) {
    }

    template <typename T, typename U>
    void defaultExtract(const T* in, U* out) {
        *out = *in;
    }

public:
    Renderer(ECS* ecs) : ecs(ecs) {
        ecs->registerComponentListener<RenderInstance, Renderer, &Renderer::onComponentAdded, &Renderer::onComponentRemoved>(this);
        ecs->registerUpdateCallback<Renderer, &Renderer::update, UpdateOrder::POST_FRAME>(this);
    }

    template <Component T, typename U = T, void (*Extract)(const T*, U*) = &Renderer::defaultExtract<T, U>>
    uint32_t createGroup(Material* material, const VertexLayoutInfo& layout) {
        return groups.emplace(material, layout, ComponentMetadata::typeOf<T>(), reinterpret_cast<void (*)(const void*, void*)>(Extract), sizeof(U));
    }

    uint32_t createObject(uint32_t group) {
        RenderGroup& g = groups[group];
        return group << 16u | g.objects.emplace(g.layout);
    }

    RenderObject& getObject(uint32_t objectID) {
        return groups[objectID >> 16u].objects[objectID & 0xFFFFu].object;
    }

    void update(double dt) {
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
            const ComponentMetadata& meta = ComponentRegistry::metadata[group.componentType];
            void* buffer = _mm_malloc(meta.size, meta.alignment);
            for (uint32_t j = 0u; j < group.objects.size(); j++) {
                Render& object = group.objects.arr[j];
                uint32_t size = object.instances.size() * group.dataSize;
                if (size == 0u) {
                    continue;
                }
                uint8_t* instanceBuffer = alloc<uint8_t>(size);
                for (uint32_t k = 0u; k < object.instances.size(); k++) {
                    uint32_t instanceID = object.instances[k];
                    RenderInstance& instance = instanceStorage.get(instanceID);
                    if (instance.componentID != UINT32_MAX) {
                        ecs->reflectRead(group.componentType, instance.componentID, buffer);
                        group.extract(buffer, instanceBuffer + k * group.dataSize);
                    }
                }
                object.object.instanceVbo.uploadData(instanceBuffer, size);
                object.object.instanceCount = object.instances.size();
                free(instanceBuffer);
            }
            _mm_free(buffer);
        }
    }

    void render(RenderMode mode) {
        for (uint32_t i = 0u; i < groups.size(); i++) {
            RenderGroup& group = groups.arr[i];
            group.material->use(mode);
            for (uint32_t i = 0u; i < group.objects.size(); i++) {
                group.objects.arr[i].object.render();
            }
        }
    }
};

#endif
