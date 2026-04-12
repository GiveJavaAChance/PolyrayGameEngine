#ifndef COMPONENTREF_H_INCLUDED
#define COMPONENTREF_H_INCLUDED

#pragma once

#include <ecs/Component.h>

template<Component T>
struct ComponentRef {
    uint32_t ID;
};

#endif