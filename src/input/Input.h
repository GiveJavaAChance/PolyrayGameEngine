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
    DISABLED = GLFW_CURSOR_DISABLED,
    CAPTURED = GLFW_CURSOR_CAPTURED
};

enum JoystickAxis : uint8_t {
    LEFT_X = 0u,
    LEFT_Y = 1u,
    RIGHT_X = 2u,
    RIGHT_Y = 3u
};

enum Joystick : uint8_t {
    LEFT = 0u,
    RIGHT = 2u
};

namespace Input {
    namespace Internal {
        constexpr static uint32_t KEY_COUNT = GLFW_KEY_LAST + 1u;
        constexpr static uint32_t KEY_WORDS = (KEY_COUNT + 63u) >> 6u;

        constexpr static uint32_t CONTROLLER_COUNT = GLFW_JOYSTICK_LAST + 1u;

        inline uint64_t keyStates[KEY_WORDS];

        inline uint8_t mouseButtonStates;

        inline vec2 mousePos;
        inline vec2 mouseDelta;

        inline vec2 mouseDragOrigin;
        inline vec2 mouseDragDelta;

        inline uint64_t controllerPresent = 0ull;
        inline uint64_t controllerButtonStates[CONTROLLER_COUNT];
        inline vec4 controllerAxesStates[CONTROLLER_COUNT];

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
        for (uint32_t i = 0u; i < KEY_WORDS; i++) {
            keyStates[i] = 0ull;
        }
        mouseButtonStates = 0u;
        eventQueue.clear();
    }

    inline void moveMouse(vec2 newPos) {
        if (mousePos.x == newPos.x && mousePos.y == newPos.y) {
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

    inline void scroll(float amt) {
        eventQueue.add(InputEvent{MOUSE_SCROLL_EVENT, {.scrollEvent = {amt}}});
    }

    inline vec2 getMousePosition() {
        return mousePos;
    }

    inline uint32_t getControllers(uint32_t controllers[CONTROLLER_COUNT]) {
        uint32_t idx = 0u;
        for(uint32_t i = 0u; i < CONTROLLER_COUNT; i++) {
            if(controllerPresent & (1ull << i)) {
                controllers[idx++] = i;
            }
        }
        return idx;
    }

    inline bool getControllerButton(uint32_t controller, uint32_t button) {
        return controllerButtonStates[controller] & (1ull << button);
    }

    inline float getControllerAxis(uint32_t controller, JoystickAxis axis) {
        return controllerAxesStates[controller][axis];
    }

    inline vec2 getControllerJoystickPosition(uint32_t controller, Joystick joystick) {
        vec4 axes = controllerAxesStates[controller];
        return prvl::vec2(axes[joystick], axes[joystick + 1u]);
    }

    inline void pollEvents(EventBus* eventBus = nullptr) {
        glfwPollEvents();
        for (uint32_t i = 0u; i < CONTROLLER_COUNT; i++) {
            if((controllerPresent & (1ull << i)) == 0ull) {
                continue;
            }
            uint64_t buttonStates = controllerButtonStates[i];
            int count = 0;
            const unsigned char* buttons = glfwGetJoystickButtons(i, &count);
            if (count != 0 && buttons) {
                for (int j = 0; j < count; j++) {
                    uint64_t mask = 1ull << j;
                    if (buttons[j] == GLFW_PRESS) {
                        buttonStates |= mask;
                    } else {
                        buttonStates &= ~mask;
                    }
                }
            }
            controllerButtonStates[i] = buttonStates;
            const float* axes = glfwGetJoystickAxes(i, &count);
            vec4 axesStates = prvl::vec4();
            if(count != 0 && axes) {
                for (int j = 0; j < count; j++) {
                    axesStates[j] = axes[j];
                }
            }
            controllerAxesStates[i] = axesStates;
        }
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
        glfwSetJoystickCallback([](int jid, int event) {
            uint64_t mask = 1ull << jid;
            if (event == GLFW_CONNECTED) {
                controllerPresent |= mask;
            } else if (event == GLFW_DISCONNECTED) {
                controllerPresent &= ~mask;
            }
        });
        for(uint32_t i = 0u; i < CONTROLLER_COUNT; i++) {
            if (glfwJoystickPresent(i)) {
                controllerPresent |= 1ull << i;
            }
        }
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
