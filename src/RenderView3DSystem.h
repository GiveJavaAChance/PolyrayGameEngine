#ifndef RENDERVIEW3DSYSTEM_H_INCLUDED
#define RENDERVIEW3DSYSTEM_H_INCLUDED

#pragma once

#include <cstdint>

#include <BindingRegistry.h>
#include <Camera3DSystem.h>
#include <Viewport.h>
#include <World.h>
#include <prvl.h>
#include <shader/ShaderManager.h>
#include <structure/UnorderedRegistry.h>

#include <gpu_types/GpuCamera3D.h>

struct RenderView3DSystem {
private:
    struct RenderView {
        Viewport* viewport;
        uint32_t cameraID;
    };

    ECS* ecs;

    UnorderedRegistry<RenderView> views;

    uint32_t bufferCapacity;
    uint32_t bufferStride;
    DynamicArray<uint8_t> cameraUploadBuffer;

    void update(double dt) {
        cameraUploadBuffer.clear();
        uint32_t size = views.arr.size() * bufferStride;
        cameraUploadBuffer.ensureCapacity(size);
        uint8_t* data = cameraUploadBuffer.data();
        for (uint32_t i = 0u; i < views.arr.size(); i++) {
            RenderView& view = views.arr[i];
            Camera3D* cam = ecs->getPtr<Camera3D>(view.cameraID);
            vec2 size = prvl::vec2(view.viewport->size);
            mat4 projection = cam->getProjection(size);
            mat4 inverseProjection = inverse(projection);

            uint32_t idx = i * bufferStride;
            std::memcpy(data + idx, &cam->cameraTransform, sizeof(mat4));
            std::memcpy(data + idx + sizeof(mat4), &cam->inverseCameraTransform, sizeof(mat4));
            std::memcpy(data + idx + 2u * sizeof(mat4), &projection, sizeof(mat4));
            std::memcpy(data + idx + 3u * sizeof(mat4), &inverseProjection, sizeof(mat4));
            std::memcpy(data + idx + 4u * sizeof(mat4), &cam->cameraPos, sizeof(vec3));
        }
        if (cameraUploadBuffer.capacity() > bufferCapacity) {
            bufferCapacity = cameraUploadBuffer.capacity();
            cameraBuffer.setSize(bufferCapacity);
        }
        cameraBuffer.uploadPartialData(data, size, 0u);
    }

public:
    GLuint cameraBinding;
    ShaderBuffer cameraBuffer;

    RenderView3DSystem(ECS* ecs) : ecs(ecs), bufferCapacity(0u), cameraBinding(BindingRegistry::allocateBufferBinding()), cameraBuffer(GL_DYNAMIC_DRAW) {
        ecs->registerUpdateCallback<RenderView3DSystem, &RenderView3DSystem::update, UpdateOrder::POST_FRAME>(this);
        ShaderManager::setValue("CAM3D_IDX", cameraBinding);

        GLint alignment;
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
        bufferStride = (sizeof(GpuCamera3D) + alignment - 1u) / alignment * alignment;
    }

    uint32_t createView(Viewport* viewport, uint32_t cameraID) {
        return views.emplace(viewport, cameraID);
    }

    void use(uint32_t view) {
        RenderView& rv = views[view];
        uint32_t loc = views.reg[view];
        glBindBufferRange(GL_UNIFORM_BUFFER, cameraBinding, cameraBuffer.ID, loc * bufferStride, sizeof(GpuCamera3D));
        rv.viewport->use();
    }
};

#endif