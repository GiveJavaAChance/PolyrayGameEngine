#ifndef RENDERVIEW3DSYSTEM_H_INCLUDED
#define RENDERVIEW3DSYSTEM_H_INCLUDED

#include "Profiler.h"
#include "rendering/DynamicShaderBuffer.h"
#pragma once

#include <cstdint>

#include <BindingRegistry.h>
#include <Camera3DSystem.h>
#include <RenderView.h>
#include <Viewport.h>
#include <World.h>
#include <prvl.h>
#include <shader/ShaderManager.h>
#include <structure/UnorderedRegistry.h>

#include <gpu_types/GpuCamera3D.h>

struct RenderView3DSystem {
private:
    ECS* ecs;

    UnorderedRegistry<RenderView> views;

    uint32_t bufferStride;
    DynamicShaderBuffer cameraBuffer;

    void update(double dt) {
        PROFILE_SCOPE(RenderView3DSystem_Update)
        uint32_t dataSize = views.arr.size() * bufferStride;
        cameraBuffer.resize(dataSize);
        uint8_t* data = cameraBuffer.bufferMapping;
        for (uint32_t i = 0u; i < views.arr.size(); i++) {
            RenderView& rv = views.arr[i];
            Camera3D* cam = ecs->getPtr<Camera3D>(rv.cameraID);
            ivec2 pos = rv.viewportRegionPos;
            ivec2 size = rv.viewportRegionSize;
            ivec2 extents = prvl::ivec2(rv.viewport->size()) - pos;
            if (size.x == -1) {
                size.x = extents.x;
            }
            if (size.y == -1) {
                size.y = extents.y;
            }
            mat4 projection = cam->getProjection(prvl::vec2(size));
            mat4 inverseProjection = inverse(projection);

            uint32_t idx = i * bufferStride;
            std::memcpy(data + idx, &cam->cameraTransform, sizeof(mat4));
            std::memcpy(data + idx + sizeof(mat4), &cam->inverseCameraTransform, sizeof(mat4));
            std::memcpy(data + idx + 2u * sizeof(mat4), &projection, sizeof(mat4));
            std::memcpy(data + idx + 3u * sizeof(mat4), &inverseProjection, sizeof(mat4));
            std::memcpy(data + idx + 4u * sizeof(mat4), &cam->cameraPos, sizeof(vec3));
        }
    }

public:
    GLuint cameraBinding;

    RenderView3DSystem(ECS* ecs)
        : ecs(ecs), bufferStride(ShaderBuffer::getUniformBufferStride(sizeof(GpuCamera3D))),
          cameraBuffer(16ull * bufferStride), cameraBinding(BindingRegistry::allocateBufferBinding()) {
        ecs->registerUpdateCallback<RenderView3DSystem, &RenderView3DSystem::update, UpdateOrder::POST_FRAME>(this);
        ShaderManager::setValue("CAM3D_IDX", cameraBinding);
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
        glBindBufferRange(GL_UNIFORM_BUFFER, cameraBinding, cameraBuffer.buffer.ID, loc * bufferStride, sizeof(GpuCamera3D));
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