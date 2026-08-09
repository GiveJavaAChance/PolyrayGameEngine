#ifndef SCENEDATA_H_INCLUDED
#define SCENEDATA_H_INCLUDED

#pragma once

#include <cstdint>
#include <unordered_map>

#include <World.h>
#include <scene/2d/Scene2D.h>
#include <scene/3d/Scene3D.h>

struct SceneData {
private:
    uint8_t* data;
    uint32_t size;

    Entity readEntity(ByteReader& reader, ECS& ecs, World* world) {
        Entity e = ecs.createEntity();
        uint32_t componentCount = reader.read<uint32_t>();
        for (uint32_t j = 0u; j < componentCount; j++) {
            uint32_t componentType = reader.read<uint32_t>();
            const ComponentMetadata& metadata = ComponentRegistry::metadata[componentType];
            metadata.deserializer(world, e, reader);
        }
        return e;
    }

    template <typename Scene>
    void instantiateImpl(ByteReader& reader, World* world, uint32_t fromNode) {
        ECS& ecs = world->ecs;
        Scene* scene = world->getSystem<Scene>();

        std::vector<uint32_t> stack;

        if (fromNode != UINT32_MAX) {
            stack.push_back(fromNode);
        }
        while (reader.pos < reader.length) {
            uint8_t instruction = reader.read<uint8_t>();
            if (instruction == 0u) {
                Entity e = readEntity(reader, ecs, world);
                if (stack.size() == 0u) {
                    stack.push_back(scene->setRootNode(e));
                } else {
                    stack.push_back(scene->addNode(stack.back(), e));
                }
            } else {
                stack.pop_back();
            }
        }
    }

public:
    SceneData(uint8_t* d, uint32_t s) noexcept : data(d), size(s) {
    }

    ~SceneData() noexcept {
        free(data);
    }

    SceneData(const SceneData&) = delete;
    SceneData& operator=(const SceneData&) = delete;

    SceneData(SceneData&& other) noexcept : data(other.data), size(other.size) {
        other.data = nullptr;
        other.size = 0u;
    }

    SceneData& operator=(SceneData&& other) noexcept {
        if (this != &other) {
            free(data);
            data = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0u;
        }
        return *this;
    }

    void instantiate(World* world, uint32_t fromNode) {
        ByteReader reader{data, size};

        uint8_t dimension = reader.read<uint8_t>();

        if (dimension == 2u) {
            instantiateImpl<Scene2D>(reader, world, fromNode);
        } else {
            instantiateImpl<Scene3D>(reader, world, fromNode);
        }
    }

    inline uint8_t getDimension() {
        return data[0u];
    }

    inline void* getData() {
        return data;
    }

    inline uint32_t getSize() {
        return size;
    }
};

#endif
