#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED
#include "glad/glad.h"

#pragma once

#include <unordered_map>

#include <BindingRegistry.h>
#include <MaterialType.h>
#include <Mesh.h>
#include <PipelineStateFlags.h>
#include <Profiler.h>
#include <RenderGroupInfo.h>
#include <RenderInstance.h>
#include <ecs/ComponentMetadata.h>
#include <ecs/ECS.h>
#include <rendering/DynamicShaderBuffer.h>
#include <shader/ShaderManager.h>
#include <structure/DynamicArray.h>
#include <structure/UnorderedRegistry.h>
#include <structure/VirtualArray.h>
#include <utils/prvl_assert.h>

struct Renderer {
private:
    struct DrawElementsIndirectCommand {
        uint32_t count;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t baseVertex;
        uint32_t baseInstance;
    };

    struct RenderObject {
        uint32_t materialID;

        uint32_t count;
        uint32_t firstIndex;
        uint32_t baseVertex;
        uint32_t baseInstance;

        DynamicArray<uint32_t> instances;

        RenderObject(uint32_t materialID, uint32_t count, uint32_t firstIndex, uint32_t baseVertex)
            : materialID(materialID), count(count), firstIndex(firstIndex), baseVertex(baseVertex) {
        }
    };

    struct PipelineStateGroup {
        uint64_t pipelineStateFlags;

        UnorderedRegistry<RenderObject> objects;

        uint32_t drawCommandOffset;

        PipelineStateGroup(uint64_t pipelineStateFlags) : pipelineStateFlags(pipelineStateFlags) {
        }
    };
    struct RenderGroup {
        RenderGroupInfo info;

        GLuint vao;

        DynamicShaderBuffer vertexBuffer;
        DynamicShaderBuffer indexBuffer;
        DynamicShaderBuffer instanceBuffer;

        DynamicShaderBuffer drawCommandBuffer;

        bool dirty = false;

        Registry materialInstanceRegistry;
        DynamicShaderBuffer materialInstanceBuffer;

        UnorderedRegistry<PipelineStateGroup> pipelineStateGroups;

        RenderGroup(const RenderGroupInfo& info)
            : info(info), vao(ShaderManager::createVAO(info.materialType.vertexLayout)),
              vertexBuffer(1ull << 20u, GL_DYNAMIC_STORAGE_BIT),
              indexBuffer(1ull << 20u, GL_DYNAMIC_STORAGE_BIT) {
            ShaderManager::setVAOBuffers(vao, info.materialType.vertexLayout, {vertexBuffer.buffer.ID, instanceBuffer.buffer.ID});
            glVertexArrayElementBuffer(vao, indexBuffer.buffer.ID);
        }
    };

    ECS* ecs;

    UnorderedRegistry<RenderGroup> groups;
    std::unordered_map<GLuint, uint32_t> groupMap;

    void onComponentAdded(Entity e, uint32_t id) {
        Storage<RenderInstance>& instanceStorage = ecs->view<RenderInstance>();
        RenderInstance& i = instanceStorage.get(id);
        uint32_t groupID = static_cast<uint32_t>(i.objectID >> 48u);
        uint32_t pipelineStateID = static_cast<uint32_t>((i.objectID >> 32u) & 0xFFFFull);
        uint32_t objectID = static_cast<uint32_t>(i.objectID & 0xFFFFFFFFull);
        RenderGroup& rg = groups[groupID];
        PipelineStateGroup& psg = rg.pipelineStateGroups[pipelineStateID];
        psg.objects[objectID].instances.add(id);
        if (!ecs->reflectGetComponentID(rg.info.componentType, e.entityID, i.componentID)) {
            std::cerr << "Could not find instance." << std::endl;
        }
    }

    void onComponentRemoved(Entity e, uint32_t id) {
        Storage<RenderInstance>& instanceStorage = ecs->view<RenderInstance>();
        RenderInstance& i = instanceStorage.get(id);
        uint32_t groupID = static_cast<uint32_t>(i.objectID >> 48u);
        uint32_t pipelineStateID = static_cast<uint32_t>((i.objectID >> 32u) & 0xFFFFull);
        uint32_t objectID = static_cast<uint32_t>(i.objectID & 0xFFFFFFFFull);
        RenderGroup& rg = groups[groupID];
        PipelineStateGroup& psg = rg.pipelineStateGroups[pipelineStateID];
        DynamicArray<uint32_t>& instances = psg.objects[objectID].instances;
        for (uint32_t i = 0u; i < instances.size(); i++) {
            if (instances[i] == id) {
                instances[i] = instances[instances.size() - 1u];
                instances.removeEnd(1u);
                return;
            }
        }
    }

public:
    GLuint materialInstanceBinding;

    Renderer(ECS* ecs) : ecs(ecs), materialInstanceBinding(BindingRegistry::allocateBufferBinding()) {
        ecs->registerComponentListener<RenderInstance, Renderer, &Renderer::onComponentAdded, &Renderer::onComponentRemoved>(this);
        ecs->registerUpdateCallback<Renderer, &Renderer::update, UpdateOrder::POST_FRAME>(this);
        ShaderManager::setValue("MAT_INS_IDX", materialInstanceBinding);
    }

    uint32_t getOrCreateGroup(const RenderGroupInfo& groupInfo) {
        GLuint programID = groupInfo.materialType.forward.ID;
        std::unordered_map<GLuint, uint32_t>::iterator it = groupMap.find(programID);
        if (it == groupMap.end()) {
            uint32_t groupID = groups.emplace(groupInfo);
            groupMap[programID] = groupID;
            return groupID;
        }
        return it->second;
    }

    template <typename T>
    uint32_t addMaterialInstance(uint32_t group, const T& instance) {
        PRVL_ASSERT(groups.reg.valid(group), "Invalid group ID")
        RenderGroup& rg = groups[group];
        uint32_t materialID = rg.materialInstanceRegistry.create();
        uint32_t location = rg.materialInstanceRegistry.locations[materialID];
        if (materialID != location) {
            uint8_t* dst = rg.materialInstanceBuffer.bufferMapping + location * sizeof(T);
            std::memcpy(dst, &instance, sizeof(T));
        } else {
            rg.materialInstanceBuffer.add(&instance, sizeof(T));
        }
        return materialID;
    }

    uint32_t getOrCreatePipelineState(uint32_t group, uint64_t pipelineStateFlags) {
        PRVL_ASSERT(groups.reg.valid(group), "Invalid group ID")
        RenderGroup& rg = groups[group];
        for (uint32_t i = 0u; i < rg.pipelineStateGroups.size(); i++) {
            PipelineStateGroup& psg = rg.pipelineStateGroups.arr[i];
            if (psg.pipelineStateFlags == pipelineStateFlags) {
                return rg.pipelineStateGroups.reg.IDs[i];
            }
        }
        return rg.pipelineStateGroups.emplace(pipelineStateFlags);
    }

    uint64_t createObject(uint32_t group, uint32_t materialID, uint32_t pipelineStateID, uint32_t vertexCount, uint32_t indexCount) {
        PRVL_ASSERT(groups.reg.valid(group), "Invalid group ID")
        RenderGroup& rg = groups[group];
        PRVL_ASSERT(rg.pipelineStateGroups.reg.valid(pipelineStateID), "Invalid pipeline state ID")
        PipelineStateGroup& psg = rg.pipelineStateGroups[pipelineStateID];

        uint32_t firstIndex = static_cast<uint32_t>(rg.indexBuffer.pos / sizeof(uint32_t));
        rg.indexBuffer.reserve(indexCount * sizeof(uint32_t));

        size_t vertexSize = static_cast<size_t>(rg.info.materialType.vertexLayout.vboStrides[0u]);
        uint32_t baseVertex = static_cast<uint32_t>(rg.vertexBuffer.pos / vertexSize);
        rg.vertexBuffer.reserve(vertexCount * vertexSize);

        uint32_t objectID = psg.objects.emplace(materialID, indexCount, firstIndex, baseVertex);
        return static_cast<uint64_t>(group) << 48u | static_cast<uint64_t>(pipelineStateID) << 32u | static_cast<uint64_t>(objectID);
    }

    uint64_t createObject(uint32_t group, uint32_t materialID, uint32_t pipelineStateID, const void* vertexData, uint32_t vertexCount, const uint32_t* indexData, uint32_t indexCount) {
        PRVL_ASSERT(groups.reg.valid(group), "Invalid group ID")
        RenderGroup& rg = groups[group];
        PRVL_ASSERT(rg.pipelineStateGroups.reg.valid(pipelineStateID), "Invalid pipeline state ID")
        PipelineStateGroup& psg = rg.pipelineStateGroups[pipelineStateID];

        uint32_t firstIndex = static_cast<uint32_t>(rg.indexBuffer.pos / sizeof(uint32_t));
        rg.indexBuffer.add(indexData, indexCount * sizeof(uint32_t));

        size_t vertexSize = static_cast<size_t>(rg.info.materialType.vertexLayout.vboStrides[0u]);
        uint32_t baseVertex = static_cast<uint32_t>(rg.vertexBuffer.pos / vertexSize);
        rg.vertexBuffer.add(vertexData, vertexCount * vertexSize);

        uint32_t objectID = psg.objects.emplace(materialID, indexCount, firstIndex, baseVertex);
        return static_cast<uint64_t>(group) << 48u | static_cast<uint64_t>(pipelineStateID) << 32u | static_cast<uint64_t>(objectID);
    }

    template <typename V>
    uint64_t createObject(uint32_t group, uint32_t materialID, uint32_t pipelineStateID, const Mesh<V>& mesh) {
        PRVL_ASSERT(groups.reg.valid(group), "Invalid group ID")
        RenderGroup& rg = groups[group];
        PRVL_ASSERT(rg.pipelineStateGroups.reg.valid(pipelineStateID), "Invalid pipeline state ID")
        PipelineStateGroup& psg = rg.pipelineStateGroups[pipelineStateID];

        uint32_t firstIndex = static_cast<uint32_t>(rg.indexBuffer.pos / sizeof(uint32_t));
        rg.indexBuffer.add(mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t));

        uint32_t baseVertex = static_cast<uint32_t>(rg.vertexBuffer.pos / sizeof(V));
        rg.vertexBuffer.add(mesh.vertices.data(), mesh.vertices.size() * sizeof(V));

        uint32_t objectID = psg.objects.emplace(materialID, mesh.indices.size(), firstIndex, baseVertex);
        return static_cast<uint64_t>(group) << 48u | static_cast<uint64_t>(pipelineStateID) << 32u | static_cast<uint64_t>(objectID);
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
            RenderGroup& rg = groups.arr[i];

            const RenderGroupInfo& info = rg.info;
            const ComponentMetadata& meta = ComponentRegistry::metadata[info.componentType];
            void* buffer = _mm_malloc(meta.size, meta.alignment);

            uint32_t totalInstanceCount = 0u;
            uint32_t totalDrawCount = 0u;
            for (uint32_t j = 0u; j < rg.pipelineStateGroups.size(); j++) {
                PipelineStateGroup& psg = rg.pipelineStateGroups.arr[j];
                psg.drawCommandOffset = totalDrawCount * sizeof(DrawElementsIndirectCommand);
                totalDrawCount += psg.objects.size();
                for (uint32_t k = 0u; k < psg.objects.size(); k++) {
                    RenderObject& o = psg.objects.arr[k];
                    o.baseInstance = totalInstanceCount;
                    totalInstanceCount += o.instances.size();
                }
            }

            size_t instanceSize = rg.info.memberSize + 4ull;

            rg.instanceBuffer.resize(totalInstanceCount * instanceSize, false);
            rg.drawCommandBuffer.resize(totalDrawCount * sizeof(DrawElementsIndirectCommand), false);

            uint8_t* drawCommandData = rg.drawCommandBuffer.bufferMapping;
            uint32_t drawIdx = 0u;
            for (uint32_t j = 0u; j < rg.pipelineStateGroups.size(); j++) {
                PipelineStateGroup& psg = rg.pipelineStateGroups.arr[j];
                for (uint32_t k = 0u; k < psg.objects.size(); k++) {
                    RenderObject& o = psg.objects.arr[k];
                    uint32_t materialInstanceIdx = rg.materialInstanceRegistry[o.materialID];
                    uint8_t* data = rg.instanceBuffer.bufferMapping + o.baseInstance * instanceSize;
                    for (uint32_t k = 0u; k < o.instances.size(); k++) {
                        uint32_t instanceID = o.instances[k];
                        RenderInstance& instance = instanceStorage.get(instanceID);
                        void* src = ecs->reflectGetPtr(info.componentType, instance.componentID);
                        if (src == nullptr) {
                            src = buffer;
                            ecs->reflectRead(info.componentType, instance.componentID, src);
                        }
                        uint8_t* dst = data + k * (info.memberSize + 4u);
                        std::memcpy(dst, &materialInstanceIdx, 4u);
                        std::memcpy(dst + 4u, reinterpret_cast<uint8_t*>(src) + info.memberOffset, info.memberSize);
                    }

                    DrawElementsIndirectCommand command;
                    command.count = o.count;
                    command.instanceCount = o.instances.size();
                    command.firstIndex = o.firstIndex;
                    command.baseVertex = o.baseVertex;
                    command.baseInstance = o.baseInstance;
                    std::memcpy(drawCommandData + drawIdx * sizeof(DrawElementsIndirectCommand), &command, sizeof(DrawElementsIndirectCommand));
                    drawIdx++;
                }
            }

            if (rg.vertexBuffer.resized || rg.instanceBuffer.resized) {
                rg.vertexBuffer.resized = false;
                rg.instanceBuffer.resized = false;
                ShaderManager::setVAOBuffers(rg.vao, rg.info.materialType.vertexLayout, {rg.vertexBuffer.buffer.ID, rg.instanceBuffer.buffer.ID});
            }
            if (rg.indexBuffer.resized) {
                rg.indexBuffer.resized = false;
                glVertexArrayElementBuffer(rg.vao, rg.indexBuffer.buffer.ID);
            }
            _mm_free(buffer);
        }
    }

    void render(RenderMode mode) {
        uint64_t currentPipelineStateFlags = 0ull;
        bool first = true;
        for (uint32_t i = 0u; i < groups.size(); i++) {
            RenderGroup& rg = groups.arr[i];
            rg.info.materialType.use(mode);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, materialInstanceBinding, rg.materialInstanceBuffer.buffer.ID);
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, rg.drawCommandBuffer.buffer.ID);
            glBindVertexArray(rg.vao);
            for (uint32_t i = 0u; i < rg.pipelineStateGroups.size(); i++) {
                PipelineStateGroup& psg = rg.pipelineStateGroups.arr[i];
                if (first) {
                    currentPipelineStateFlags = ~psg.pipelineStateFlags;
                    first = false;
                }
                uint64_t diff = currentPipelineStateFlags ^ psg.pipelineStateFlags;
                if (diff & PipelineStateFlags::BACKFACE_CULLING) {
                    if (psg.pipelineStateFlags & PipelineStateFlags::BACKFACE_CULLING) {
                        glEnable(GL_CULL_FACE);
                    } else {
                        glDisable(GL_CULL_FACE);
                    }
                }
                if (diff & PipelineStateFlags::ALPHA_BLEND) {
                    if (psg.pipelineStateFlags & PipelineStateFlags::ALPHA_BLEND) {
                        glEnable(GL_BLEND);
                    } else {
                        glDisable(GL_BLEND);
                    }
                }
                if (diff & PipelineStateFlags::ALPHA_TO_COVER) {
                    if (psg.pipelineStateFlags & PipelineStateFlags::ALPHA_TO_COVER) {
                        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
                    } else {
                        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
                    }
                }
                glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*) static_cast<uintptr_t>(psg.drawCommandOffset), psg.objects.size(), 0ll);
            }
        }
    }
};
#endif