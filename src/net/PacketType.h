#ifndef PACKETTYPE_H_INCLUDED
#define PACKETTYPE_H_INCLUDED

#pragma once

#include <functional>

#include <net/PacketReader.h>
#include <net/PacketWriter.h>

struct PacketType {
    std::function<void(PacketWriter)> serializer;
    std::function<void(uint32_t, PacketReader)> deserializer;

    PacketType(std::function<void(PacketWriter)> ser, std::function<void(uint32_t, PacketReader)> des) : serializer(std::move(ser)), deserializer(std::move(des)) {
    }
};

#endif
