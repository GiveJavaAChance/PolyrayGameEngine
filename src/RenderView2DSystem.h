#ifndef RENDERVIEW2DSYSTEM_H_INCLUDED
#define RENDERVIEW2DSYSTEM_H_INCLUDED

#pragma once

#include <cstdint>

#include <BindingRegistry.h>
#include <Camera2DSystem.h>
#include <Profiler.h>
#include <RenderView.h>
#include <Viewport.h>
#include <World.h>
#include <prvl.h>
#include <shader/ShaderManager.h>
#include <structure/UnorderedRegistry.h>

#include <gpu_types/GpuCamera2D.h>

struct RenderView2DSystem {
private:
    ECS* ecs;

    UnorderedRegistry<RenderView> views;

    uint32_t bufferCapacity;
    uint32_t bufferStride;
    DynamicArray<uint8_t> cameraUploadBuffer;

    void update(double dt) {
        PROFILE_SCOPE(RenderView2DSystem_Update)
        cameraUploadBuffer.clear();
        uint32_t dataSize = views.arr.size() * bufferStride;
        cameraUploadBuffer.ensureCapacity(dataSize);
        uint8_t* data = cameraUploadBuffer.data();
        GpuCamera2D cameraData;
        for (uint32_t i = 0u; i < views.arr.size(); i++) {
            RenderView& rv = views.arr[i];
            Camera2D* cam = ecs->getPtr<Camera2D>(rv.cameraID);
            ivec2 pos = rv.viewportRegionPos;
            ivec2 size = rv.viewportRegionSize;
            ivec2 extents = prvl::ivec2(rv.viewport->size()) - pos;
            if (size.x == -1) {
                size.x = extents.x;
            }
            if (size.y == -1) {
                size.y = extents.y;
            }
            cameraData.cameraTransform = prvl::mat3x4(cam->cameraTransform);
            cameraData.inverseCameraTransform = prvl::mat3x4(cam->inverseCameraTransform);
            mat3 projection = cam->getProjection(prvl::vec2(size));
            cameraData.projection = prvl::mat3x4(projection);
            cameraData.inverseProjection = prvl::mat3x4(inverse(projection));

            uint32_t idx = i * bufferStride;
            std::memcpy(data + idx, &cameraData, sizeof(GpuCamera2D));
        }
        if (cameraUploadBuffer.capacity() > bufferCapacity) {
            bufferCapacity = cameraUploadBuffer.capacity();
            cameraBuffer.setSize(bufferCapacity);
        }
        cameraBuffer.uploadPartialData(data, dataSize, 0u);
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

    uint32_t createView(Viewport* viewport, uint32_t cameraID, ivec2 viewportRegionPos = {0, 0}, ivec2 viewportRegionSize = {-1, -1}) {
        return views.emplace(viewport, cameraID, viewportRegionPos, viewportRegionSize);
    }

    RenderView& getView(uint32_t view) {
        return views[view];
    }

    void use(uint32_t view) {
        RenderView& rv = views[view];
        uint32_t loc = views.reg[view];
        glBindBufferRange(GL_UNIFORM_BUFFER, cameraBinding, cameraBuffer.ID, loc * bufferStride, sizeof(GpuCamera2D));
        rv.viewport->use();
        ivec2 pos = rv.viewportRegionPos;
        ivec2 size = rv.viewportRegionSize;
        ivec2 extents = prvl::ivec2(rv.viewport->size()) - pos;
        if (size.x == -1) {
            size.x = extents.x;
        }
        if (size.y == -1) {
            size.y = extents.y;
        }
        glViewport(pos.x, pos.y, size.x, size.y);
        glScissor(pos.x, pos.y, size.x, size.y);
    }
};

#endif