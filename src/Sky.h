#ifndef SKY_H_INCLUDED
#define SKY_H_INCLUDED

#pragma once

#include <FullscreenQuad.h>

struct Sky {
    FullscreenQuad quad;

    Sky(uint32_t fragmentShader) : quad(ShaderManager::createProgram({ShaderManager::compileShaderFile("res/shaders/Sky.vert", GL_VERTEX_SHADER), fragmentShader})) {
    }

    Sky() : Sky(ShaderManager::compileShaderFile("res/shaders/Sky.frag", GL_FRAGMENT_SHADER)) {
    }

    void render() {
        quad.render();
    }
};

#endif