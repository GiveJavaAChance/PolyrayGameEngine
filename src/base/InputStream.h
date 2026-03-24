#ifndef INPUTSTREAM_H_INCLUDED
#define INPUTSTREAM_H_INCLUDED

#pragma once
#include <cstddef>
#include <cstdint>

class InputStream {
public:
    virtual ~InputStream() = default;

    virtual int read() = 0;

    virtual int64_t read(uint8_t* buffer, size_t length) = 0;

    virtual bool available() = 0;

    virtual void close() = 0;
};

#endif
