#ifndef SCENE3DNODEUPDATEDEVENT_H_INCLUDED
#define SCENE3DNODEUPDATEDEVENT_H_INCLUDED

#pragma once

#include <cstdint>

struct Transform3D;

struct Scene3DNodeUpdatedEvent {
    uint32_t node;
    uint32_t entityID;
    Transform3D* tx;
};

#endif