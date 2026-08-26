#ifndef RENDERMODE_H_INCLUDED
#define RENDERMODE_H_INCLUDED

#pragma once

#include <cstdint>

enum RenderMode : uint8_t {
    FORWARD,
    DEFERRED,
    DEPTH
};

#endif