#ifndef GLFWINDOW_H_INCLUDED
#define GLFWINDOW_H_INCLUDED

#pragma once
#include "base/glad/glad.h"
#include <GLFW/glfw3.h>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <iostream>
#include "Input.h"

struct GLFWindow {
private:
    uint32_t width = 0u;
    uint32_t height = 0u;
    const char* name;
    GLFWwindow* window = nullptr;

    int32_t exitKey = GLFW_KEY_Q;

    int32_t button = -1;
    float mx = 0.0f, my = 0.0f;

public:
    explicit GLFWindow(const char* title) : name(title) {}

    ~GLFWindow() {
        exit();
    }

    void createFrame(const uint32_t w, const uint32_t h, const bool title, const bool fullscreen, const bool exclusive) {
        if (!glfwInit()) {
            std::cerr << "Unable to initialize GLFW" << std::endl;
            std::exit(1);
        }

        if (fullscreen) {
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            if (!mode) {
                std::cerr << "Failed to get primary monitor video mode" << std::endl;
            } else {
                width = mode->width;
                height = mode->height;
            }
        } else {
            width = w;
            height = h;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        if (fullscreen && exclusive) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            window = glfwCreateWindow(width, height, name, monitor, nullptr);
            if (!window) {
                std::cerr << "Failed to create the GLFW window" << std::endl;
                std::exit(1);
            }
        } else {
            int titleHint = title ? GLFW_TRUE : GLFW_FALSE;
            glfwWindowHint(GLFW_RESIZABLE, titleHint);
            glfwWindowHint(GLFW_DECORATED, titleHint);
            glfwWindowHint(GLFW_MAXIMIZED, titleHint);

            window = glfwCreateWindow(width, height, name, nullptr, nullptr);

            if (!window) {
                std::cerr << "Failed to create the GLFW window" << std::endl;
                std::exit(1);
            }
            if (fullscreen && !title) {
                glfwSetWindowPos(window, 0, 0);
            }
        }

        glfwSetWindowUserPointer(window, this);

        setupCallbacks();

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        glfwShowWindow(window);
        glfwFocusWindow(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            std::exit(1);
        }

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    inline GLFWwindow* getWindow() const {
        return window;
    }

    inline void update() {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    inline void hideCursor(bool hide) {
        glfwSetInputMode(window, GLFW_CURSOR, hide ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
    }

    inline void getMousePos(double& x, double& y) {
        glfwGetCursorPos(window, &x, &y);
    }

    inline void setMousePos(double x, double y) {
        glfwSetCursorPos(window, x, y);
    }

    inline void setExitShortcut(int32_t key) {
        exitKey = key;
    }

    inline bool isWindowOpen() const {
        return !glfwWindowShouldClose(window);
    }

    inline bool isWindowFocused() const {
        return glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE;
    }

    void exit() {
        if (window) {
            glfwSetKeyCallback(window, nullptr);
            glfwSetCharCallback(window, nullptr);
            glfwSetMouseButtonCallback(window, nullptr);
            glfwSetCursorPosCallback(window, nullptr);
            glfwSetScrollCallback(window, nullptr);
            glfwSetJoystickCallback(nullptr);

            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }

    inline uint32_t getWidth() const {
        return width;
    }

    inline uint32_t getHeight() const {
        return height;
    }

    void (*keyPress)(int32_t) = nullptr;
    void (*keyRelease)(int32_t) = nullptr;

    void (*mousePress)(float, float, int32_t) = nullptr;
    void (*mouseRelease)(float, float, int32_t) = nullptr;

    void (*mouseMove)(float, float) = nullptr;
    void (*mouseDrag)(float, float, float, float, int32_t) = nullptr;

    void (*scroll)(float, float, float) = nullptr;
    void (*windowResized)(uint32_t, uint32_t) = nullptr;

private:
    void setupCallbacks() {
        glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
            GLFWindow* self = static_cast<GLFWindow*>(glfwGetWindowUserPointer(win));

            if (action == GLFW_PRESS) {
                if (key == self->exitKey) {
                    glfwSetWindowShouldClose(win, GLFW_TRUE);
                }
                Input::setKey(key, true);
                if (self->keyPress) {
                    self->keyPress(key);
                }
            } else if (action == GLFW_RELEASE) {
                Input::setKey(key, false);
                if (self->keyRelease) {
                    self->keyRelease(key);
                }
            }
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow* win, int button, int action, int mods) {
            GLFWindow* self = static_cast<GLFWindow*>(glfwGetWindowUserPointer(win));

            double xpos, ypos;
            glfwGetCursorPos(win, &xpos, &ypos);
            float x = static_cast<float>(xpos);
            float y = static_cast<float>(self->height) - static_cast<float>(ypos);

            if (action == GLFW_PRESS) {
                if (self->mousePress) self->mousePress(x, y, button);
                self->button = button;
            } else if (action == GLFW_RELEASE) {
                if (self->mouseRelease) self->mouseRelease(x, y, button);
                self->button = -1;
            }
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow* win, double xpos, double ypos) {
            GLFWindow* self = static_cast<GLFWindow*>(glfwGetWindowUserPointer(win));

            float x = static_cast<float>(xpos);
            float y = static_cast<float>(self->height) - static_cast<float>(ypos);

            if (self->button != -1) {
                if (self->mouseDrag) {
                    self->mouseDrag(self->mx, self->my, x, y, self->button);
                }
            } else {
                if (self->mouseMove) {
                    self->mouseMove(x, y);
                }
            }
            self->mx = x;
            self->my = y;
        });

        glfwSetScrollCallback(window, [](GLFWwindow* win, double xOffset, double yOffset) {
            GLFWindow* self = static_cast<GLFWindow*>(glfwGetWindowUserPointer(win));

            double xpos, ypos;
            glfwGetCursorPos(win, &xpos, &ypos);
            float x = static_cast<float>(xpos);
            float y = static_cast<float>(self->height) - static_cast<float>(ypos);

            if (self->scroll) {
                self->scroll(x, y, static_cast<float>(yOffset));
            }
        });

        glfwSetWindowSizeCallback(window, [](GLFWwindow* win, int newWidth, int newHeight) {
            GLFWindow* self = static_cast<GLFWindow*>(glfwGetWindowUserPointer(win));

            self->width = static_cast<uint32_t>(newWidth);
            self->height = static_cast<uint32_t>(newHeight);
            glViewport(0, 0, newWidth, newHeight);
            if (self->windowResized) {
                self->windowResized(newWidth, newHeight);
            }
        });

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int fbWidth, int fbHeight) {
            GLFWindow* self = static_cast<GLFWindow*>(glfwGetWindowUserPointer(win));

            self->width = static_cast<uint32_t>(fbWidth);
            self->height = static_cast<uint32_t>(fbHeight);
            glViewport(0, 0, fbWidth, fbHeight);
            if (self->windowResized) {
                self->windowResized(fbWidth, fbHeight);
            }
        });
    }
};


#endif
