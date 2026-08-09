#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED

#pragma once

#include <cstdint>

#include <utils/ByteIO.h>

struct World;
struct Entity;

template <typename T>
struct Serial {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output);
    static void deserialize(World* world, Entity& e, ByteReader& input);
};

#endif