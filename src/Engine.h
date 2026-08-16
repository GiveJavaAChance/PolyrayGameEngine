#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#pragma once

#include <iostream>

#include <GLFW/glfw3.h>

namespace Engine {
    inline void init() {
        if (!glfwInit()) {
            std::cerr << "Unable to initialize GLFW" << std::endl;
            std::exit(1);
        }
    }

    inline void exit() {
        glfwTerminate();
    }
}

#endif