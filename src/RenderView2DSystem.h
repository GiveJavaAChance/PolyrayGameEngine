#ifndef RENDERVIEW2DSYSTEM_H_INCLUDED
#define RENDERVIEW2DSYSTEM_H_INCLUDED

#pragma once

#include <cstdint>

#include <BindingRegistry.h>
#include <Camera2DSystem.h>
#include <Viewport.h>
#include <World.h>
#include <prvl.h>
#include <shader/ShaderManager.h>
#include <structure/UnorderedRegistry.h>

#include <gpu_types/GpuCamera2D.h>

struct RenderView2DSystem {
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
        GpuCamera2D cameraData;
        for (uint32_t i = 0u; i < views.arr.size(); i++) {
            RenderView& view = views.arr[i];
            Camera2D* cam = ecs->getPtr<Camera2D>(view.cameraID);
            vec2 size = prvl::vec2(view.viewport->size);
            cameraData.cameraTransform = prvl::mat3x4(cam->cameraTransform);
            cameraData.inverseCameraTransform = prvl::mat3x4(cam->inverseCameraTransform);
            mat3 projection = cam->getProjection(size);
            cameraData.projection = prvl::mat3x4(projection);
            cameraData.inverseProjection = prvl::mat3x4(inverse(projection));
            
            uint32_t idx = i * bufferStride;
            std::memcpy(data + idx, &cameraData, sizeof(GpuCamera2D));
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

    RenderView2DSystem(ECS* ecs) : ecs(ecs), bufferCapacity(0u), cameraBinding(BindingRegistry::allocateBufferBinding()), cameraBuffer(GL_DYNAMIC_DRAW) {
        ecs->registerUpdateCallback<RenderView2DSystem, &RenderView2DSystem::update, UpdateOrder::POST_FRAME>(this);
        ShaderManager::setValue("CAM2D_IDX", cameraBinding);

        GLint alignment;
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
        bufferStride = (sizeof(GpuCamera2D) + alignment - 1u) / alignment * alignment;
    }

    uint32_t createView(Viewport* viewport, uint32_t cameraID) {
        return views.emplace(viewport, cameraID);
    }

    void use(uint32_t view) {
        RenderView& rv = views[view];
        uint32_t loc = views.reg[view];
        glBindBufferRange(GL_UNIFORM_BUFFER, cameraBinding, cameraBuffer.ID, loc * bufferStride, sizeof(GpuCamera2D));
        rv.viewport->use();
    }
};

#endif