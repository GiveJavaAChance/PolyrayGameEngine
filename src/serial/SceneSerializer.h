#ifndef SCENESERIALIZER_H_INCLUDED
#define SCENESERIALIZER_H_INCLUDED

#pragma once

#include <cstdint>

#include <World.h>
#include <scene/SceneData.h>
#include <utils/ByteIO.h>

struct SceneSerializer {
private:
    template <typename T>
    static void write(DynamicArray<uint8_t>& buffer, T& e) {
        buffer.addAll(reinterpret_cast<uint8_t*>(&e), sizeof(T));
    }

    template <typename T>
    static void write(DynamicArray<uint8_t>& buffer, T&& e) {
        buffer.addAll(reinterpret_cast<uint8_t*>(&e), sizeof(T));
    }

    static void writeEntity(DynamicArray<uint8_t>& buffer, World* world, uint32_t entityID) {
        ECS& ecs = world->ecs;

        uint32_t componentCount = ecs.getComponentCountRaw(entityID);
        write(buffer, componentCount);

        uint64_t* components = ecs.getComponentsRaw(entityID);
        for (uint32_t j = 0u; j < componentCount; j++) {
            uint64_t component = components[j];
            uint32_t componentID = ECS::extractComponentID(component);
            uint32_t componentType = ECS::extractComponentType(component);
            write(buffer, componentType);
            const ComponentMetadata& metadata = ComponentRegistry::metadata[componentType];
            ByteWriter writer{&buffer};
            metadata.serializer(world, componentID, writer);
        }
    }

    template <typename Scene>
    static void writeNode(DynamicArray<uint8_t>& buffer, World* world, uint32_t node) {
        Scene* scene = world->getSystem<Scene>();

        write<uint8_t>(buffer, 0u);
        writeEntity(buffer, world, scene->getEntity(node).entityID);

        uint32_t childCount = scene->getChildCount(node);

        for (uint32_t i = 0u; i < childCount; i++) {
            uint32_t child = scene->getChild(node, i);
            writeNode<Scene>(buffer, world, child);
        }
        write<uint8_t>(buffer, 1u);
    }

    template <typename Scene, uint8_t Dim>
    static SceneData serialize(World* world) {
        DynamicArray<uint8_t> buffer;

        write(buffer, Dim);

        Scene* scene = world->getSystem<Scene>();
        writeNode<Scene>(buffer, world, scene->getRootNode());

        uint8_t* out = alloc<uint8_t>(buffer.size());
        std::memcpy(out, buffer.data(), buffer.size());
        return {out, buffer.size()};
    }

public:
    inline static SceneData serialize2D(World* world) {
        return serialize<Scene2D, 2u>(world);
    }

    inline static SceneData serialize3D(World* world) {
        return serialize<Scene3D, 3u>(world);
    }
};

#endif
