#ifndef ANIMATION_H_INCLUDED
#define ANIMATION_H_INCLUDED

#pragma once

#include <string>

#include <animation/AnimationChannel.h>
#include <prvl.h>
#include <structure/DynamicArray.h>

struct Animation {
    std::string name;
    uint32_t groupID;
    float duration;

    DynamicArray<AnimationChannel> channels;
};

#endif