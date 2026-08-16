#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#pragma once

#include <cstdint>

#include <Window.h>

#include <EventBus.h>
#include <input/InputEvent.h>
#include <prvl.h>

enum MouseInputMode : uint32_t {
    NORMAL = GLFW_CURSOR_NORMAL,
    HIDDEN = GLFW_CURSOR_HIDDEN,
    CAPTURED = GLFW_CURSOR_CAPTURED,
    DISABLED = GLFW_CURSOR_DISABLED
};

namespace Input {
    namespace Internal {
        constexpr static uint32_t KEY_COUNT = GLFW_KEY_LAST + 1u;
        constexpr static uint32_t KEY_WORDS = (KEY_COUNT + 63u) >> 6u;

        inline uint64_t keyStates[KEY_WORDS];

        inline uint8_t mouseButtonStates;

        inline vec2 mousePos;
        inline vec2 mouseDelta;

        inline vec2 mouseDragOrigin;
        inline vec2 mouseDragDelta;

        inline DynamicArray<InputEvent> eventQueue;
    }

    using namespace Internal;

    inline void setMouseInputMode(const Window& window, MouseInputMode mode) {
        glfwSetInputMode(window.nativeHandle(), GLFW_CURSOR, mode);
    }

    inline void setKey(uint32_t key) {
        uint32_t u = key >> 6u;
        uint64_t mask = 1ull << (key & 63u);
        if ((keyStates[u] & mask) == 0ull) {
            eventQueue.add(InputEvent{KEY_EVENT, {.keyEvent = {key, true}}});
        }
        keyStates[u] |= mask;
    }

    inline void clearKey(uint32_t key) {
        uint32_t u = key >> 6u;
        uint64_t mask = 1ull << (key & 63u);
        if (keyStates[u] & mask) {
            eventQueue.add(InputEvent{KEY_EVENT, {.keyEvent = {key, false}}});
        }
        keyStates[u] &= ~mask;
    }

    inline bool getKey(uint32_t key) {
        uint32_t u = key >> 6u;
        uint64_t mask = 1ull << (key & 63u);
        return keyStates[u] & mask;
    }

    inline void setMouseButton(uint32_t button) {
        uint8_t mask = 1u << button;
        if ((mouseButtonStates & mask) == 0ull) {
            eventQueue.add(InputEvent{MOUSE_BUTTON_EVENT, {.mouseButtonEvent = {button, true}}});
            mouseDragOrigin = mousePos;
        }
        mouseButtonStates |= mask;
    }

    inline void clearMouseButton(uint32_t button) {
        uint8_t mask = 1u << button;
        if (mouseButtonStates & mask) {
            eventQueue.add(InputEvent{MOUSE_BUTTON_EVENT, {.mouseButtonEvent = {button, false}}});
            mouseDragDelta = prvl::vec2();
        }
        mouseButtonStates &= ~mask;
    }

    inline bool getMouseButton(uint32_t button) {
        uint8_t mask = 1u << button;
        return mouseButtonStates & mask;
    }

    inline void clearStates() {
        for(uint32_t i = 0u; i < KEY_WORDS; i++) {
            keyStates[i] = 0ull;
        }
        mouseButtonStates = 0u;
        eventQueue.clear();
    }

    inline void moveMouse(vec2 newPos) {
        if(mousePos.x == newPos.x && mousePos.y == newPos.y) {
            return;
        }
        vec2 prevPos = mousePos;
        mousePos = newPos;
        mouseDelta = mousePos - prevPos;
        mouseDragDelta += mouseDelta;
        if (mouseButtonStates) {
            eventQueue.add(InputEvent{MOUSE_DRAG_EVENT, {.mouseDragEvent = {prevPos, mousePos, mouseDragDelta, mouseDragOrigin}}});
        } else {
            eventQueue.add(InputEvent{MOUSE_MOVE_EVENT, {.mouseMoveEvent = {prevPos, mousePos, mouseDelta}}});
        }
    }

    inline void resetMousePos(vec2 newPos) {
        mousePos = newPos;
    }

    inline vec2 getMousePosition() {
        return mousePos;
    }

    inline void scroll(float amt) {
        eventQueue.add(InputEvent{MOUSE_SCROLL_EVENT, {.scrollEvent = {amt}}});
    }

    inline void pollEvents(EventBus* eventBus = nullptr) {
        glfwPollEvents();
        if (eventBus) {
            for (uint32_t i = 0u; i < eventQueue.size(); i++) {
                eventBus->fire(eventQueue[i]);
            }
        }
        eventQueue.clear();
    }

    inline void init(const Window& window) {
        GLFWwindow* handle = window.nativeHandle();
        glfwSetKeyCallback(handle, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
            if (action == GLFW_PRESS) {
                setKey(key);
            } else if (action == GLFW_RELEASE) {
                clearKey(key);
            }
        });
        glfwSetMouseButtonCallback(handle, [](GLFWwindow* win, int button, int action, int mods) {
            if (action == GLFW_PRESS) {
                setMouseButton(button);
            } else if (action == GLFW_RELEASE) {
                clearMouseButton(button);
            }
        });
        glfwSetCursorPosCallback(handle, [](GLFWwindow* win, double xpos, double ypos) {
            moveMouse(prvl::vec2(xpos, ypos));
        });
        glfwSetScrollCallback(handle, [](GLFWwindow* win, double xOffset, double yOffset) {
            scroll(static_cast<float>(yOffset));
        });
    }

    inline void exit(const Window& window) {
        GLFWwindow* handle = window.nativeHandle();
        glfwSetKeyCallback(handle, nullptr);
        glfwSetCharCallback(handle, nullptr);
        glfwSetMouseButtonCallback(handle, nullptr);
        glfwSetCursorPosCallback(handle, nullptr);
        glfwSetScrollCallback(handle, nullptr);
        glfwSetJoystickCallback(nullptr);
    }
}

#endif
