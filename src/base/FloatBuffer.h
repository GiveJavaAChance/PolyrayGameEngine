#ifndef FLOATBUFFER_H_INCLUDED
#define FLOATBUFFER_H_INCLUDED

#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>

struct FloatBuffer {
private:
    const uint32_t length;
    uint32_t offset;

public:
    float* buffer;

    explicit FloatBuffer(uint32_t length) : length(length), offset(0) {
        buffer = new float[length];
    }

    ~FloatBuffer() {
        delete[] buffer;
    }

    FloatBuffer(const FloatBuffer&) = delete;
    FloatBuffer& operator=(const FloatBuffer&) = delete;

    FloatBuffer(FloatBuffer&& other) noexcept
        : length(other.length), offset(other.offset), buffer(other.buffer) {
        other.buffer = nullptr;
        other.offset = 0;
    }

    FloatBuffer& operator=(FloatBuffer&& other) noexcept {
        if (this != &other) {
            delete[] buffer;
            buffer = other.buffer;
            offset = other.offset;
            other.buffer = nullptr;
            other.offset = 0;
        }
        return *this;
    }

    void put(float value) {
        if (offset >= length) throw std::out_of_range("FloatBuffer overflow");
        buffer[offset++] = value;
    }

    void put(const float* values, uint32_t len) {
        if (offset + len > length) throw std::out_of_range("FloatBuffer overflow");
        std::memcpy(buffer + offset, values, len * sizeof(float));
        offset += len;
    }

    void reset() {
        offset = 0;
    }

    uint32_t getOffset() const {
        return offset;
    }

    uint32_t getLength() const {
        return length;
    }
};

#endif
