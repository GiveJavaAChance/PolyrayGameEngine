#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#pragma once

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <prvl.h>

enum WindowMode : uint8_t {
    WINDOWED,
    MAXIMIZED,
    EXCLUSIVE_FULLSCREEN
};

struct Window {
private:
    uint32_t width = 0u;
    uint32_t height = 0u;
    GLFWwindow* handle = nullptr;

public:
    Window(const char* title, uint32_t width, uint32_t height, WindowMode mode, bool decorated) : width(width), height(height) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        glfwWindowHint(GLFW_MAXIMIZED, mode == MAXIMIZED && decorated);
        glfwWindowHint(GLFW_DECORATED, decorated && (mode == WINDOWED || mode == MAXIMIZED));

        if (mode == EXCLUSIVE_FULLSCREEN || (mode == MAXIMIZED && !decorated)) {
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            if (!mode) {
                std::cerr << "Failed to get primary monitor video mode" << std::endl;
            } else {
                this->width = mode->width;
                this->height = mode->height;
            }
        }
        handle = glfwCreateWindow(this->width, this->height, title, mode == EXCLUSIVE_FULLSCREEN ? glfwGetPrimaryMonitor() : nullptr, nullptr);
        if (!handle) {
            std::cerr << "Failed to create the GLFW window" << std::endl;
            std::exit(1);
        }

        if(mode == MAXIMIZED && decorated) {
            glfwSetWindowPos(handle, 0, 0);
        }

        glfwSetWindowUserPointer(handle, this);

        setupCallbacks();

        glfwMakeContextCurrent(handle);
        glfwSwapInterval(1);
        glfwShowWindow(handle);
        glfwFocusWindow(handle);

        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            std::exit(1);
        }
    }

    ~Window() {
        exit();
    }

    inline GLFWwindow* nativeHandle() const {
        return handle;
    }

    inline void update() {
        glfwSwapBuffers(handle);
    }

    inline vec2 getMousePos() {
        double x, y;
        glfwGetCursorPos(handle, &x, &y);
        return prvl::vec2(static_cast<float>(x), static_cast<float>(y));
    }

    inline void setMousePos(const vec2& p) {
        glfwSetCursorPos(handle, p.x, p.y);
    }

    inline bool isWindowOpen() const {
        return !glfwWindowShouldClose(handle);
    }

    inline bool isWindowFocused() const {
        return glfwGetWindowAttrib(handle, GLFW_FOCUSED) == GLFW_TRUE;
    }

    inline void close() {
        glfwSetWindowShouldClose(handle, true);
    }

    inline void exit() {
        if (handle) {
            glfwDestroyWindow(handle);
            handle = nullptr;
        }
    }

    inline uint32_t getWidth() const {
        return width;
    }

    inline uint32_t getHeight() const {
        return height;
    }

    void (*windowResized)(uint32_t, uint32_t) = nullptr;

private:
    void setupCallbacks() {
        glfwSetWindowSizeCallback(handle, [](GLFWwindow* win, int newWidth, int newHeight) {
            Window* w = static_cast<Window*>(glfwGetWindowUserPointer(win));
            w->width = static_cast<uint32_t>(newWidth);
            w->height = static_cast<uint32_t>(newHeight);
            if (w->windowResized) {
                w->windowResized(newWidth, newHeight);
            }
        });
        glfwSetFramebufferSizeCallback(handle, [](GLFWwindow* win, int fbWidth, int fbHeight) {
            Window* w = static_cast<Window*>(glfwGetWindowUserPointer(win));
            w->width = static_cast<uint32_t>(fbWidth);
            w->height = static_cast<uint32_t>(fbHeight);
            if (w->windowResized) {
                w->windowResized(fbWidth, fbHeight);
            }
        });
    }
};

#endif
