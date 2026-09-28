#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#pragma once

#include <cstdint>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

#include <glad/glad.h>

#include <prvl.h>
#include <utils/perf.h>

#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <windowsx.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

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

    std::function<void(double)> updateFunc;
    uint64_t lastTime;
    double dt = 1.0 / 60.0;

    uint32_t inputID;

    void updateLoop() {
        if (!updateFunc) {
            return;
        }
        updateFunc(dt);
        update();
        uint64_t time = Time::nanoTime();
        dt = static_cast<double>(time - lastTime) / 1000000000.0;
        lastTime = time;
    }

#ifdef _WIN32
    WNDPROC wndProc;

    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (!window) {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        switch (msg) {
            case WM_ENTERSIZEMOVE: {
                SetTimer(hwnd, 1, 1, nullptr);
                break;
            }
            case WM_EXITSIZEMOVE: {
                KillTimer(hwnd, 1);
                break;
            }
            case WM_TIMER: {
                if (wParam == 1) {
                    window->updateLoop();
                }
                return 0;
            }
        }
        return CallWindowProc(window->wndProc, hwnd, msg, wParam, lParam);
    }
#endif

public:
    Window(const char* title, uint32_t width, uint32_t height, WindowMode mode, bool decorated, bool alwaysOnTop = false, bool transparentBackground = false) : width(width), height(height) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        glfwWindowHint(GLFW_MAXIMIZED, mode == MAXIMIZED && decorated);
        glfwWindowHint(GLFW_DECORATED, decorated && (mode == WINDOWED || mode == MAXIMIZED));

        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, transparentBackground);

        glfwWindowHint(GLFW_FLOATING, alwaysOnTop);

        uint32_t borderlessFix = 0u;

        if (mode == EXCLUSIVE_FULLSCREEN || (mode == MAXIMIZED && !decorated)) {
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            if (!mode) {
                std::cerr << "Failed to get primary monitor video mode" << std::endl;
            } else {
                this->width = mode->width;
                this->height = mode->height;
                borderlessFix = 1u;
            }
        }
        handle = glfwCreateWindow(this->width + borderlessFix, this->height, title, mode == EXCLUSIVE_FULLSCREEN ? glfwGetPrimaryMonitor() : nullptr, nullptr);
        if (!handle) {
            std::cerr << "Failed to create the GLFW window" << std::endl;
            std::exit(1);
        }

        glfwSetWindowUserPointer(handle, this);

        setupCallbacks();

        glfwMakeContextCurrent(handle);
        glfwSwapInterval(1);
        glfwShowWindow(handle);
        glfwFocusWindow(handle);

#ifdef _WIN32
        HWND hwnd = glfwGetWin32Window(handle);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        wndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::windowProc)));
#endif

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

    inline uint32_t& getInputID() {
        return inputID;
    }

    void enableClickthrough() {
#ifdef _WIN32
        HWND hwnd = glfwGetWin32Window(handle);
        LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        exStyle |= WS_EX_LAYERED | WS_EX_TRANSPARENT;
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
#else
        std::cerr << "Mouse passthough not supported." << std::endl;
#endif
    }

    inline void run(std::function<void(double)> updateFunc) {
        this->updateFunc = updateFunc;
        lastTime = Time::nanoTime();
        while (isWindowOpen()) {
            updateLoop();
        }
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
