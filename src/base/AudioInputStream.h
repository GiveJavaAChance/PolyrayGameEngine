#ifndef AUDIOINPUTSTREAM_H_INCLUDED
#define AUDIOINPUTSTREAM_H_INCLUDED

#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include "AudioFormat.h"
#include "InputStream.h"

class AudioInputStream : public InputStream {
public:
    AudioFormat format;

    explicit AudioInputStream(InputStream& sourceStream) : source(sourceStream), dataRemaining(0), closed(false) {
        parseWavHeader();
    }

    ~AudioInputStream() override {
        if (!closed) {
            close();
        }
    }

    int64_t read(uint8_t* buffer, size_t length) override {
        if (closed) throw std::runtime_error("Stream is closed");
        if (dataRemaining == 0) return -1;

        size_t toRead = (length < dataRemaining) ? length : dataRemaining;
        size_t bytesRead = source.read(buffer, toRead);
        dataRemaining -= bytesRead;
        return bytesRead;
    }

    int read() override {
        if (closed) throw std::runtime_error("Stream is closed");
        if (dataRemaining == 0) return -1;

        int byte = source.read();
        if (byte != -1) {
            --dataRemaining;
        }
        return byte;
    }

    bool available() override {
        return (!closed) && (dataRemaining > 0) && source.available();
    }

    void close() override {
        if (!closed) {
            source.close();
            closed = true;
        }
    }

private:
    InputStream& source;
    uint32_t dataRemaining;
    bool closed;

    void parseWavHeader() {
        uint8_t header[12];
        size_t bytesRead = source.read(header, 12);
        if (bytesRead < 12) {
            throw std::runtime_error("Failed to read RIFF header");
        }

        if (std::memcmp(header, "RIFF", 4) != 0) {
            throw std::runtime_error("Invalid WAV file: missing 'RIFF'");
        }
        if (std::memcmp(header + 8, "WAVE", 4) != 0) {
            throw std::runtime_error("Invalid WAV file: missing 'WAVE'");
        }

        // Now read chunks until 'fmt ' is found
        bool fmtFound = false;
        bool dataFound = false;
        uint32_t dataChunkSize = 0;

        while (!fmtFound) {
            uint8_t chunkHeader[8];
            bytesRead = source.read(chunkHeader, 8);
            if (bytesRead < 8) {
                throw std::runtime_error("Unexpected EOF searching for 'fmt ' chunk");
            }
            uint32_t chunkSize = readLE32(chunkHeader + 4);
            if (std::memcmp(chunkHeader, "fmt ", 4) == 0) {
                // Read the fmt chunk data
                if (chunkSize < 16) {
                    throw std::runtime_error("Invalid 'fmt ' chunk size");
                }
                uint8_t fmtData[16];
                bytesRead = source.read(fmtData, 16);
                if (bytesRead < 16) {
                    throw std::runtime_error("Failed to read full 'fmt ' chunk");
                }

                uint16_t audioFormatCode = readLE16(fmtData + 0);
                if (audioFormatCode != 1) {
                    throw std::runtime_error("Unsupported WAV format: only PCM supported");
                }

                uint16_t channels = readLE16(fmtData + 2);
                uint32_t sampleRate = readLE32(fmtData + 4);
                // uint32_t byteRate = readLE32(fmtData + 8);
                // uint16_t blockAlign = readLE16(fmtData + 12);
                uint16_t bitsPerSample = readLE16(fmtData + 14);

                format = AudioFormat(sampleRate, channels, bitsPerSample);

                // If chunkSize > 16, skip remaining bytes
                if (chunkSize > 16) {
                    size_t skipBytes = chunkSize - 16;
                    uint8_t discard[256];
                    while (skipBytes > 0) {
                        size_t toRead = (skipBytes > sizeof(discard)) ? sizeof(discard) : skipBytes;
                        size_t readNow = source.read(discard, toRead);
                        if (readNow == 0) {
                            throw std::runtime_error("Unexpected EOF skipping extra fmt chunk bytes");
                        }
                        skipBytes -= readNow;
                    }
                }
                fmtFound = true;
            } else {
                // Skip this chunk
                uint8_t discard[256];
                uint32_t skipBytes = chunkSize;
                while (skipBytes > 0) {
                    size_t toRead = (skipBytes > sizeof(discard)) ? sizeof(discard) : skipBytes;
                    size_t readNow = source.read(discard, toRead);
                    if (readNow == 0) {
                        throw std::runtime_error("Unexpected EOF skipping unknown chunk");
                    }
                    skipBytes -= readNow;
                }
            }
        }

        // Now find the data chunk
        while (!dataFound) {
            uint8_t chunkHeader[8];
            bytesRead = source.read(chunkHeader, 8);
            if (bytesRead < 8) {
                throw std::runtime_error("Unexpected EOF searching for 'data' chunk");
            }
            uint32_t chunkSize = readLE32(chunkHeader + 4);
            if (std::memcmp(chunkHeader, "data", 4) == 0) {
                dataChunkSize = chunkSize;
                dataFound = true;
            } else {
                // skip chunk
                uint8_t discard[256];
                uint32_t skipBytes = chunkSize;
                while (skipBytes > 0) {
                    size_t toRead = (skipBytes > sizeof(discard)) ? sizeof(discard) : skipBytes;
                    size_t readNow = source.read(discard, toRead);
                    if (readNow == 0) {
                        throw std::runtime_error("Unexpected EOF skipping unknown chunk");
                    }
                    skipBytes -= readNow;
                }
            }
        }

        dataRemaining = dataChunkSize;
    }

    static uint16_t readLE16(const uint8_t* bytes) {
        return static_cast<uint16_t>(bytes[0]) | (static_cast<uint16_t>(bytes[1]) << 8);
    }

    static uint32_t readLE32(const uint8_t* bytes) {
        return static_cast<uint32_t>(bytes[0]) |
               (static_cast<uint32_t>(bytes[1]) << 8) |
               (static_cast<uint32_t>(bytes[2]) << 16) |
               (static_cast<uint32_t>(bytes[3]) << 24);
    }
};

#endif
