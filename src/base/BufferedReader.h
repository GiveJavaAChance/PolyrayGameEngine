#ifndef BUFFEREDREADER_H_INCLUDED
#define BUFFEREDREADER_H_INCLUDED

#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include "base/InputStream.h"

#define BUFFER_SIZE 4096

struct BufferedReader {
private:
    InputStream& stream;
    uint8_t* buffer;
    uint32_t pos;
    int64_t limit;

    bool fillBuffer() {
        pos = 0;
        limit = stream.read(buffer, BUFFER_SIZE);
        return limit > 0;
    }

public:
    BufferedReader(InputStream& stream) : stream(stream), pos(0), limit(0) {
        buffer = new uint8_t[BUFFER_SIZE]{};
    }

    ~BufferedReader() {
        delete[] buffer;
    }

    int64_t readLine(std::string& outLine) {
        outLine.clear();
        bool sawAnyChar = false;

        while (true) {
            if (pos >= limit) {
                if (!fillBuffer()) {
                    return sawAnyChar ? static_cast<int64_t>(outLine.size()) : -1;
                }
            }

            uint32_t start = pos;
            while (pos < limit) {
                uint8_t ch = buffer[pos];
                if (ch == '\n' || ch == '\r') {
                    if (pos > start) {
                        outLine.append(reinterpret_cast<char*>(buffer + start), pos - start);
                        sawAnyChar = true;
                    }

                    pos++;
                    if (ch == '\r' && pos < limit && buffer[pos] == '\n') {
                        pos++;
                    }

                    return static_cast<int64_t>(outLine.size());
                }

                pos++;
            }

            if (pos > start) {
                outLine.append(reinterpret_cast<char*>(buffer + start), pos - start);
                sawAnyChar = true;
            }
        }
    }

    std::string readAll() {
        std::ostringstream result;
        std::string line;
        while (readLine(line) != -1) {
            result << line << '\n';
        }
        return result.str();
    }
};

#endif
