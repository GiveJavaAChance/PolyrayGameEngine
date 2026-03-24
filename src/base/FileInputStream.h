#ifndef FILEINPUTSTREAM_H_INCLUDED
#define FILEINPUTSTREAM_H_INCLUDED

#pragma once
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdint>
#include <string>
#include "base/InputStream.h"

class FileInputStream : public InputStream {
private:
    std::ifstream file;
    std::string filename;
    bool open;

public:
    FileInputStream(const std::string& fileName) : filename(fileName), open(false) {
        file.open(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::ios_base::failure("Failed to open file: " + filename);
        }
        open = true;
    }

    ~FileInputStream() {
        if (open) {
            close();
        }
    }

    int read() override {
        if (!open) {
            throw std::ios_base::failure("File is not open");
        }

        uint8_t byte;
        if (file.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
            return static_cast<int>(byte);
        } else {
            return -1;
        }
    }

    int64_t read(uint8_t* buffer, size_t length) override {
        if (!open) {
            throw std::ios_base::failure("File is not open");
        }

        file.read(reinterpret_cast<char*>(buffer), length);
        std::streamsize count = file.gcount();

        return (count > 0) ? count : -1;
    }

    bool available() override {
        return file.peek() != EOF;
    }

    void close() override {
        if (open) {
            file.close();
            open = false;
        }
    }

    bool isOpen() const {
        return open;
    }

    std::streampos tell() {
        return file.tellg();
    }

    void seek(std::streampos pos) {
        file.seekg(pos);
        if (!file) {
            throw std::ios_base::failure("Failed to seek to position");
        }
    }

    std::streampos size() {
        std::streampos currentPos = file.tellg();
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(currentPos);
        return fileSize;
    }
};

#endif

