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
    size_t size;

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
    uint32_t instantiateImpl(ByteReader& reader, World* world, uint32_t fromNode) {
        ECS& ecs = world->ecs;
        Scene* scene = world->getSystem<Scene>();

        DynamicArray<uint32_t> stack;

        uint32_t newNode = UINT32_MAX;
        bool first = true;

        if (fromNode != UINT32_MAX) {
            stack.add(fromNode);
        }
        DynamicArray<char> nameBuffer;
        while (reader.pos < reader.size) {
            uint8_t instruction = reader.read<uint8_t>();
            if (instruction == 0u) {
                uint32_t nameLength = reader.read<uint32_t>();
                nameBuffer.ensureCapacity(nameLength + 1u);
                reader.read(nameBuffer.data(), nameLength);
                nameBuffer[nameLength] = 0;
                Entity e = readEntity(reader, ecs, world);
                uint32_t n;
                if (stack.size() == 0u) {
                    n = scene->setRootNode(e, nameBuffer.data());
                } else {
                    n = scene->addNode(stack[stack.size() - 1u], e, nameBuffer.data());
                }
                stack.add(n);
                if (first) {
                    first = false;
                    newNode = n;
                }
            } else {
                stack.removeEnd(1u);
            }
        }
        return newNode;
    }

public:
    SceneData(uint8_t* data, size_t size) noexcept : data(data), size(size) {
    }

    SceneData() noexcept : data(nullptr), size(0ull) {
    }

    ~SceneData() noexcept {
        if (data) {
            free(data);
        }
    }

    SceneData(const SceneData&) = delete;
    SceneData& operator=(const SceneData&) = delete;

    SceneData(SceneData&& other) noexcept : data(other.data), size(other.size) {
        other.data = nullptr;
        other.size = 0ull;
    }

    SceneData& operator=(SceneData&& other) noexcept {
        if (this != &other) {
            if (data) {
                free(data);
            }
            data = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0ull;
        }
        return *this;
    }

    uint32_t instantiate(World* world, uint32_t fromNode) {
        ByteReader reader{data, size};

        uint8_t dimension = reader.read<uint8_t>();

        if (dimension == 2u) {
            return instantiateImpl<Scene2D>(reader, world, fromNode);
        }
        return instantiateImpl<Scene3D>(reader, world, fromNode);
    }

    inline uint8_t getDimension() const {
        return data[0u];
    }

    inline const uint8_t* getData() const {
        return data;
    }

    inline uint32_t getSize() const {
        return size;
    }
};

#endif
