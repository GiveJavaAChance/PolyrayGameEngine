#ifndef PACKETWRITER_H_INCLUDED
#define PACKETWRITER_H_INCLUDED

#pragma once

#include <bit>
#include <cstdint>

#include <structure/DynamicArray.h>

#include <utils/net_order.h>

struct PacketWriter {
private:
    DynamicArray<uint8_t>& data;

public:
    PacketWriter(DynamicArray<uint8_t>& data) : data(data) {
    }

    template <typename T>
    inline void write(T v) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        reverse_bytes<T>(v);
#endif
        data.addAll(reinterpret_cast<uint8_t*>(&v), sizeof(T));
    }

    template <typename T>
    inline void writeAll(T* v, uint32_t count) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        for (uint32_t i = 0u; i < count; i++) {
            T e = v[i];
            reverse_bytes<T>(e);
            data.addAll(reinterpret_cast<uint8_t*>(&e), sizeof(T));
        }
#else
        data.addAll(reinterpret_cast<uint8_t*>(v), sizeof(T) * count);
#endif
    }
};

#endif
