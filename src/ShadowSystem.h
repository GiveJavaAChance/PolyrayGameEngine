#ifndef SHADOWSYSTEM_H_INCLUDED
#define SHADOWSYSTEM_H_INCLUDED

#pragma once

#include <BindingRegistry.h>
#include <Renderer.h>
#include <rendering/GLGBuffer.h>
#include <rendering/GLTexture.h>

#include <gpu_types/GpuCamera3D.h>

#define SHADOW_SIZE 1024u

struct ShadowSystem {
private:
    struct Shadow {
        mat4 lightSpaceTransform;
        uvec2 shadowMapOffset;
        uvec2 shadowMapSize;
    };

public:
    uint32_t shadowsX;
    uint32_t shadowsY;
    uint32_t maxShadowCount;

    GLTexture shadowAtlas;
    GLGBuffer shadowBuffer;

    ShaderBuffer shadowCamBuffer;

    uint32_t cameraBufferStride;
    ShaderBuffer cameraBuffer;

    UnorderedRegistry<GpuCamera3D> shadowCameras;

    ShadowSystem(uint32_t shadowsX, uint32_t shadowsY)
        : shadowsX(shadowsX), shadowsY(shadowsY), maxShadowCount(shadowsX * shadowsY),
          shadowAtlas(GLTexture::createTexture2D(SHADOW_SIZE * shadowsX, SHADOW_SIZE * shadowsY, GL_DEPTH_COMPONENT32)),
          shadowBuffer(std::initializer_list<GLTexture*>{}, &shadowAtlas),
          shadowCamBuffer(maxShadowCount * sizeof(Shadow)),
          cameraBufferStride(ShaderBuffer::getUniformBufferStride(sizeof(GpuCamera3D))),
          cameraBuffer(maxShadowCount * cameraBufferStride) {
        shadowAtlas.setInterpolation(true);
        glTextureParameteri(shadowAtlas.ID, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
        glTextureParameteri(shadowAtlas.ID, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        shadowAtlas.setWrapMode(GL_CLAMP_TO_BORDER);
        float borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
        glTextureParameterfv(shadowAtlas.ID, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindTextureUnit(32u, shadowAtlas.ID);

        ShaderManager::setValue("SHADOW3D_IDX", BindingRegistry::bindBufferBase(shadowCamBuffer, GL_SHADER_STORAGE_BUFFER));
    }

    uint32_t createOrthographicShadowCaster(float width, float depth) {
        uint32_t id = shadowCameras.emplace();
        GpuCamera3D& cam = shadowCameras[id];
        cam.projection = orthographicProjection(width, 1.0f, -depth * 0.5f, depth * 0.5f);
        cam.inverseProjection = inverse(cam.projection);
        return id;
    }

    uint32_t createPerspectiveShadowCaster(float fov, float nearZ) {
        uint32_t id = shadowCameras.emplace();
        GpuCamera3D& cam = shadowCameras[id];
        cam.projection = reverseZPerspectiveProjection(fov, 1.0f, nearZ);
        cam.inverseProjection = inverse(cam.projection);
        return id;
    }

    void removeShadowCaster(uint32_t shadowId) {
        shadowCameras.remove(shadowId);
    }

    void updateShadowCamera(uint32_t shadowId, const mat4& transform) {
        GpuCamera3D& cam = shadowCameras[shadowId];
        cam.cameraPos = prvl::vec3(transform[3]);
        cam.inverseCameraTransform = prvl::mat4(prvl::mat3(transform));
        cam.cameraTransform = transpose(cam.inverseCameraTransform);

        mat4 tr = diag(prvl::vec4(1.0f));
        tr[3] = prvl::vec4(-cam.cameraPos, 1.0f);

        uint32_t i = shadowCameras.reg[shadowId];
        uint32_t shadowX = (i % shadowsX) * SHADOW_SIZE;
        uint32_t shadowY = (i / shadowsX) * SHADOW_SIZE;
        Shadow shadow{cam.projection * cam.cameraTransform * tr, {shadowX, shadowY}, {SHADOW_SIZE, SHADOW_SIZE}};
        shadowCamBuffer.uploadData(&shadow, sizeof(Shadow), i * sizeof(Shadow));
        cameraBuffer.uploadData(&cam, sizeof(GpuCamera3D), i * cameraBufferStride);
    }

    void renderShadows(GLuint cameraBinding, Renderer* renderer) {
        shadowBuffer.bind();
        glEnable(GL_DEPTH_TEST);
        for (uint32_t i = 0u; i < shadowCameras.size(); i++) {
            uint32_t shadowX = (i % shadowsX) * SHADOW_SIZE;
            uint32_t shadowY = (i / shadowsX) * SHADOW_SIZE;

            glViewport(shadowX, shadowY, SHADOW_SIZE, SHADOW_SIZE);
            glEnable(GL_SCISSOR_TEST);
            glScissor(shadowX, shadowY, SHADOW_SIZE, SHADOW_SIZE);
            glClear(GL_DEPTH_BUFFER_BIT);
            glDisable(GL_SCISSOR_TEST);

            glBindBufferRange(GL_UNIFORM_BUFFER, cameraBinding, cameraBuffer.ID, i * cameraBufferStride, sizeof(GpuCamera3D));
            renderer->render(RenderMode::DEPTH);
        }
    }
};

#endif
