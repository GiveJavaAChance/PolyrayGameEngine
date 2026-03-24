#ifndef BUFFEREDINPUTSTREAM_H_INCLUDED
#define BUFFEREDINPUTSTREAM_H_INCLUDED

#pragma once
#include "base/InputStream.h"
#define BUFFER_SIZE 4096

class BufferedInputStream : public InputStream {
private:
    InputStream& input;
    uint8_t* buffer;
    size_t bufferPos = 0;
    int64_t bufferCount = 0;
    bool closed = false;

public:
    BufferedInputStream(InputStream& in) : input(in) {
        buffer = new uint8_t[BUFFER_SIZE];
    }

    ~BufferedInputStream() {
        delete[] buffer;
    }

    int read() override {
        if (closed) throw std::ios_base::failure("Stream closed");

        if (bufferPos >= bufferCount) {
            bufferCount = input.read(buffer, BUFFER_SIZE);
            bufferPos = 0;
            if (bufferCount == -1) return -1;
        }

        return buffer[bufferPos++];
    }

    int64_t read(uint8_t* outBuffer, size_t length) override {
        if (closed) throw std::ios_base::failure("Stream closed");

        int64_t totalRead = 0;

        while (length > 0) {
            if (bufferPos >= bufferCount) {
                bufferCount = input.read(buffer, BUFFER_SIZE);
                bufferPos = 0;
                if (bufferCount == -1) break;
            }

            size_t available = bufferCount - bufferPos;
            size_t toCopy = (length < available) ? length : available;

            std::memcpy(outBuffer + totalRead, buffer + bufferPos, toCopy);
            bufferPos += toCopy;
            totalRead += toCopy;
            length -= toCopy;
        }

        return (totalRead > 0) ? totalRead : -1;
    }

    bool available() override {
        if (closed) return false;
        if (bufferPos < bufferCount) return true;
        return input.available();
    }

    void close() override {
        if (!closed) {
            input.close();
            closed = true;
        }
    }

    bool isOpen() const {
        return !closed;
    }
};

#endif
