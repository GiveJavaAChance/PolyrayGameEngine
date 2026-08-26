#ifndef SCENE2DNODEUPDATEDEVENT_H_INCLUDED
#define SCENE2DNODEUPDATEDEVENT_H_INCLUDED

#pragma once

#include <cstdint>

struct Transform2D;

struct Scene2DNodeUpdatedEvent {
    uint32_t node;
    uint32_t entityID;
    Transform2D* tx;
};

#endif