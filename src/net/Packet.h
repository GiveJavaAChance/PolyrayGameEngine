#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED

#pragma once

#include <cstdint>

constexpr uint32_t MAX_PACKET_SIZE = 1024u;

struct Packet {
    uint32_t clientID;
    uint32_t packetID;
    void* data;
};

#endif