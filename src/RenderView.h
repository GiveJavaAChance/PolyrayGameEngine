#ifndef RENDERVIEW_H_INCLUDED
#define RENDERVIEW_H_INCLUDED

#pragma once

#include <Viewport.h>

struct RenderView {
    Viewport* viewport;
    uint32_t cameraID;
    ivec2 viewportRegionPos;
    ivec2 viewportRegionSize;
};

#endif