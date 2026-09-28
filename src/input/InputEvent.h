#ifndef INPUTEVENT_H_INCLUDED
#define INPUTEVENT_H_INCLUDED

#pragma once

#include <cstdint>

#include <prvl.h>

#undef KEY_EVENT

enum InputEventType : uint8_t {
    KEY_EVENT,
    MOUSE_BUTTON_EVENT,
    MOUSE_MOVE_EVENT,
    MOUSE_DRAG_EVENT,
    MOUSE_SCROLL_EVENT,
    CONTROLLER_BUTTON_EVENT
};

struct Window;

struct InputEvent {
    InputEventType type;
    union {
        struct {
            Window* window;
            uint32_t key;
            bool pressed;
        } keyEvent;
        struct {
            Window* window;
            uint32_t button;
            bool pressed;
        } mouseButtonEvent;
        struct {
            Window* window;
            vec2 from;
            vec2 to;
            vec2 delta;
        } mouseMoveEvent;
        struct {
            Window* window;
            vec2 from;
            vec2 to;
            vec2 delta;
            vec2 origin;
        } mouseDragEvent;
        struct {
            Window* window;
            float amt;
        } scrollEvent;
        struct {
            uint32_t controller;
            uint32_t button;
            bool pressed;
        } controllerButtonEvent;
    };
};

#endif