#ifndef TEXTRENDERER_H_INCLUDED
#define TEXTRENDERER_H_INCLUDED

#pragma once

#include <cstdint>
#include <array>

#include <base/glad/glad.h>
#include <structure/DynamicArray.h>
#include <rendering/GLTexture.h>
#include <rendering/ShaderBuffer.h>
#include <rendering/ShaderBufferHeap.h>
#include <structure/UnorderedRegistry.h>

#define INTS_PER_STRING 2

struct TextRenderer {
private:
    ShaderBufferHeap stringBuffer;
    GLuint stringBufferBinding;
    GLTexture* bitmap;
    char firstChar;
    uint32_t characterWidth;
    uint32_t characterHeight;
    ShaderBuffer drawBuffer;
    ShaderBuffer vbo;

    ShaderProgram shader;
    GLuint vao;

    DynamicArray<uint32_t> lengths;

    bool doUpdate = true;

    uint32_t getChar(const char* c, uint32_t idx, uint32_t length) {
        return idx >= length ? 0u : static_cast<uint32_t>(c[idx] - firstChar);
    }

public:
    TextRenderer(uint32_t bytes, GLTexture* bitmap, char startingChar, float* verts, uint32_t characterWidth, uint32_t characterHeight) :
        stringBuffer(bytes >> 2u, 1u), stringBufferBinding(BindingRegistry::bindBufferBase(stringBuffer.buffer, GL_SHADER_STORAGE_BUFFER)),
        bitmap(bitmap), firstChar(startingChar), characterWidth(characterWidth), characterHeight(characterHeight), drawBuffer(GL_DYNAMIC_DRAW),
        vbo(GL_STATIC_DRAW) {
        vbo.uploadData(verts, 16u);
        ShaderManager::setValue("STR_IDX", stringBufferBinding);
        ShaderManager::setValue("INTS_PER_STRING", INTS_PER_STRING);
        ShaderManager::setValue("CHARACTER_WIDTH", characterWidth);
        shader = ShaderManager::createProgram({ShaderManager::compileShaderFile("Text.vert", GL_VERTEX_SHADER), ShaderManager::compileShaderFile("Text.frag", GL_FRAGMENT_SHADER)});
        vao = ShaderManager::createVAO(shader, {vbo.ID});
    }

    TextRenderer(uint32_t bytes, GLTexture* bitmap, char startingChar, vec2 rectMin, vec2 rectMax) : TextRenderer(bytes, bitmap, startingChar, std::array<float, 16>{
            rectMin.x, rectMin.y, 0.0f, 0.0f,
            rectMax.x, rectMin.y, 1.0f, 0.0f,
            rectMin.x, rectMax.y, 0.0f, 1.0f,
            rectMax.x, rectMax.y, 1.0f, 1.0f
        }.data(), static_cast<uint32_t>(abs(rectMax.x - rectMin.x)), static_cast<uint32_t>(abs(rectMax.y - rectMin.y))) {
    }

    TextRenderer(uint32_t bytes, GLTexture* bitmap, char startingChar, bool flipY) : TextRenderer(bytes, bitmap, startingChar, vec2(0.0f, flipY ? 0.0f : bitmap->height), vec2(bitmap->width, flipY ? bitmap->height : 0.0f)) {
    }

    void update() {
        if (!doUpdate) {
            return;
        }
        doUpdate = false;
        std::unordered_set<uint32_t>& active = stringBuffer.getActive();
        UnorderedRegistry<uint64_t>& strings = stringBuffer.getAllocations();
        uint32_t* drawData = alloc<uint32_t>(active.size() << 2u);
        uint32_t idx = 0u;
        for (uint32_t ID : active) {
            drawData[idx++] = 4u;
            drawData[idx++] = lengths[ID];
            drawData[idx++] = 0u;
            drawData[idx++] = static_cast<uint32_t>(strings[ID] >> 32ull);
        }
        drawBuffer.uploadData(drawData, active.size() << 2u);
        stringBuffer.pollUpdates();
    }

    uint32_t push(const std::string& str, uint32_t color, int16_t x, int16_t y) {
        const char* chars = str.c_str();
        uint32_t length = ((str.size() + 3u) >> 2u) + INTS_PER_STRING;
        uint32_t packedPos = static_cast<uint32_t>((x + 32768) & 0xFFFF) << 16u | static_cast<uint32_t>((y + 32768) & 0xFFFF);
        uint32_t* data = alloc<uint32_t>(length);
        data[0] = packedPos;
        data[1] = color;
        for (uint32_t i = 0u; i < length - INTS_PER_STRING; i++) {
            uint32_t idx = i << 2u;
            uint32_t packedChars = getChar(chars, idx, str.size()) << 24u | getChar(chars, idx + 1u, str.size()) << 16u | getChar(chars, idx + 2u, str.size()) << 8u | getChar(chars, idx + 3u, str.size());
            data[i + INTS_PER_STRING] = packedChars;
        }
        uint32_t ID = stringBuffer.allocate(data, length);
        lengths.add(str.size());
        doUpdate = true;
        return ID;
    }

    uint32_t pushCentered(const std::string& str, uint32_t color, int16_t x, int16_t y) {
        return push(str, color, x - ((str.size() * characterWidth) >> 1), y - (characterHeight >> 1));
    }

    void pop(uint32_t ID) {
        uint32_t idx = stringBuffer.getAllocations().reg[ID];
        stringBuffer.free(ID);
        lengths[idx] = lengths[lengths.size() - 1u];
        lengths.removeEnd(1u);
        doUpdate = true;
    }

    void setVisible(uint32_t ID, bool visible) {
        if (visible) {
            stringBuffer.activate(ID);
        } else {
            stringBuffer.deactivate(ID);
        }
        doUpdate = true;
    }

    void modify(uint32_t ID, uint32_t newColor, int16_t newX, int16_t newY) {
        uint32_t packedPos = static_cast<uint32_t>((newX + 32768) & 0xFFFF) << 16u | static_cast<uint32_t>((newY + 32768) & 0xFFFF);
        uint32_t data[]{packedPos, newColor};
        stringBuffer.write(ID, 0u, data, 2u);
        doUpdate = true;
    }

    void render() {
        shader.use();
        glBindTextureUnit(0, bitmap->ID);
        glBindVertexArray(vao);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawBuffer.ID);
        glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, 0, stringBuffer.getActiveCount(), 16);
    }
};

#endif
