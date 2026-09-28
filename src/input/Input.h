#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#pragma once

#include <cstdint>

#include <EventBus.h>
#include <Window.h>
#include <input/InputEvent.h>
#include <prvl.h>
#include <structure/UnorderedRegistry.h>

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

        struct WindowInputState {
            Window* window;

            uint64_t keyStates[KEY_WORDS];

            uint8_t mouseButtonStates = 0u;

            vec2 mousePos = prvl::vec2();
            vec2 mouseDelta = prvl::vec2();

            vec2 mouseDragOrigin = prvl::vec2();
            vec2 mouseDragDelta = prvl::vec2();
        };

        inline UnorderedRegistry<WindowInputState> windowInputStates;

        inline uint64_t controllerPresent = 0ull;
        inline uint64_t controllerButtonStates[CONTROLLER_COUNT];
        inline vec4 controllerAxesStates[CONTROLLER_COUNT];

        inline DynamicArray<InputEvent> eventQueue;
    }

    using namespace Internal;

    inline void setMouseInputMode(const Window& window, MouseInputMode mode) {
        glfwSetInputMode(window.nativeHandle(), GLFW_CURSOR, mode);
    }

    inline void setKey(uint32_t key, Window* window = nullptr) {
        WindowInputState& windowState = window ? windowInputStates[window->getInputID()] : windowInputStates.arr[0u];
        uint32_t u = key >> 6u;
        uint64_t mask = 1ull << (key & 63u);
        if ((windowState.keyStates[u] & mask) == 0ull) {
            eventQueue.add(InputEvent{KEY_EVENT, {.keyEvent = {window, key, true}}});
        }
        windowState.keyStates[u] |= mask;
    }

    inline void clearKey(uint32_t key, Window* window = nullptr) {
        WindowInputState& windowState = window ? windowInputStates[window->getInputID()] : windowInputStates.arr[0u];
        uint32_t u = key >> 6u;
        uint64_t mask = 1ull << (key & 63u);
        if (windowState.keyStates[u] & mask) {
            eventQueue.add(InputEvent{KEY_EVENT, {.keyEvent = {window, key, false}}});
        }
        windowState.keyStates[u] &= ~mask;
    }

    inline bool getKey(uint32_t key, Window* window = nullptr) {
        WindowInputState& windowState = window ? windowInputStates[window->getInputID()] : windowInputStates.arr[0u];
        uint32_t u = key >> 6u;
        uint64_t mask = 1ull << (key & 63u);
        return windowState.keyStates[u] & mask;
    }

    inline void setMouseButton(uint32_t button, Window* window = nullptr) {
        WindowInputState& windowState = window ? windowInputStates[window->getInputID()] : windowInputStates.arr[0u];
        uint8_t mask = 1u << button;
        if ((windowState.mouseButtonStates & mask) == 0ull) {
            eventQueue.add(InputEvent{MOUSE_BUTTON_EVENT, {.mouseButtonEvent = {window, button, true}}});
            windowState.mouseDragOrigin = windowState.mousePos;
        }
        windowState.mouseButtonStates |= mask;
    }

    inline void clearMouseButton(uint32_t button, Window* window = nullptr) {
        WindowInputState& windowState = window ? windowInputStates[window->getInputID()] : windowInputStates.arr[0u];
        uint8_t mask = 1u << button;
        if (windowState.mouseButtonStates & mask) {
            eventQueue.add(InputEvent{MOUSE_BUTTON_EVENT, {.mouseButtonEvent = {window, button, false}}});
            windowState.mouseDragDelta = prvl::vec2();
        }
        windowState.mouseButtonStates &= ~mask;
    }

    inline bool getMouseButton(uint32_t button, Window* window = nullptr) {
        WindowInputState& windowState = window ? windowInputStates[window->getInputID()] : windowInputStates.arr[0u];
        uint8_t mask = 1u << button;
        return windowState.mouseButtonStates & mask;
    }

    inline void moveMouse(vec2 newPos, Window* window = nullptr) {
        WindowInputState& windowState = window ? windowInputStates[window->getInputID()] : windowInputStates.arr[0u];
        if (windowState.mousePos.x == newPos.x && windowState.mousePos.y == newPos.y) {
            return;
        }
        vec2 prevPos = windowState.mousePos;
        windowState.mousePos = newPos;
        windowState.mouseDelta = windowState.mousePos - prevPos;
        windowState.mouseDragDelta += windowState.mouseDelta;
        if (windowState.mouseButtonStates) {
            eventQueue.add(InputEvent{MOUSE_DRAG_EVENT, {.mouseDragEvent = {window, prevPos, windowState.mousePos, windowState.mouseDragDelta, windowState.mouseDragOrigin}}});
        } else {
            eventQueue.add(InputEvent{MOUSE_MOVE_EVENT, {.mouseMoveEvent = {window, prevPos, windowState.mousePos, windowState.mouseDelta}}});
        }
    }

    inline void scroll(float amt, Window* window = nullptr) {
        eventQueue.add(InputEvent{MOUSE_SCROLL_EVENT, {.scrollEvent = {window, amt}}});
    }

    inline vec2 getMousePosition(Window* window = nullptr) {
        WindowInputState& windowState = window ? windowInputStates[window->getInputID()] : windowInputStates.arr[0u];
        return windowState.mousePos;
    }

    inline uint32_t getControllers(uint32_t controllers[CONTROLLER_COUNT]) {
        uint32_t idx = 0u;
        for (uint32_t i = 0u; i < CONTROLLER_COUNT; i++) {
            if (controllerPresent & (1ull << i)) {
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
            if ((controllerPresent & (1ull << i)) == 0ull) {
                continue;
            }
            uint64_t buttonStates = controllerButtonStates[i];
            int count = 0;
            const unsigned char* buttons = glfwGetJoystickButtons(i, &count);
            if (count != 0 && buttons) {
                for (uint32_t j = 0u; j < static_cast<uint32_t>(count); j++) {
                    uint64_t mask = 1ull << j;
                    bool on = buttonStates & mask;
                    if (buttons[j] == GLFW_PRESS) {
                        buttonStates |= mask;
                        if (!on) {
                            eventQueue.add(InputEvent{CONTROLLER_BUTTON_EVENT, {.controllerButtonEvent = {i, j, true}}});
                        }
                    } else {
                        buttonStates &= ~mask;
                        if (on) {
                            eventQueue.add(InputEvent{CONTROLLER_BUTTON_EVENT, {.controllerButtonEvent = {i, j, false}}});
                        }
                    }
                }
            }
            controllerButtonStates[i] = buttonStates;
            const float* axes = glfwGetJoystickAxes(i, &count);
            vec4 axesStates = prvl::vec4();
            if (count != 0 && axes) {
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

    inline void initWindowInput(Window* window) {
        GLFWwindow* handle = window->nativeHandle();
        window->getInputID() = windowInputStates.emplace(window);
        glfwSetKeyCallback(handle, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
            Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
            if (action == GLFW_PRESS) {
                setKey(key, w);
            } else if (action == GLFW_RELEASE) {
                clearKey(key, w);
            }
        });
        glfwSetMouseButtonCallback(handle, [](GLFWwindow* win, int button, int action, int mods) {
            Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
            if (action == GLFW_PRESS) {
                setMouseButton(button, w);
            } else if (action == GLFW_RELEASE) {
                clearMouseButton(button, w);
            }
        });
        glfwSetCursorPosCallback(handle, [](GLFWwindow* win, double xpos, double ypos) {
            Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
            moveMouse(prvl::vec2(xpos, ypos), w);
        });
        glfwSetScrollCallback(handle, [](GLFWwindow* win, double xOffset, double yOffset) {
            Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
            scroll(static_cast<float>(yOffset), w);
        });
    }

    inline void initControllerInput() {
        glfwSetJoystickCallback([](int jid, int event) {
            uint64_t mask = 1ull << jid;
            if (event == GLFW_CONNECTED) {
                controllerPresent |= mask;
            } else if (event == GLFW_DISCONNECTED) {
                controllerPresent &= ~mask;
            }
        });
        for (uint32_t i = 0u; i < CONTROLLER_COUNT; i++) {
            if (glfwJoystickPresent(i)) {
                controllerPresent |= 1ull << i;
            }
        }
    }

    inline void exit() {
        DynamicArray<WindowInputState>& windowStates = windowInputStates.arr;
        for (uint32_t i = 0u; i < windowStates.size(); i++) {
            GLFWwindow* handle = windowStates[i].window->nativeHandle();
            glfwSetKeyCallback(handle, nullptr);
            glfwSetCharCallback(handle, nullptr);
            glfwSetMouseButtonCallback(handle, nullptr);
            glfwSetCursorPosCallback(handle, nullptr);
            glfwSetScrollCallback(handle, nullptr);
        }
        glfwSetJoystickCallback(nullptr);
    }
}

#endif
