#ifndef TYPEREG_H_INCLUDED
#define TYPEREG_H_INCLUDED

#pragma once

#include <cstdint>

#define TYPE_REGISTRY(name)             \
struct name {                           \
private:                                \
    inline static uint32_t nextID = 0;  \
public:                                 \
    template<typename T>                \
    static uint32_t getTypeId() {       \
        static uint32_t ID = nextID++;  \
        return ID;                      \
    }                                   \
};

#endif
