#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#pragma once

#include <mutex>
#include <string>

#include <net/SocketHandle.h>

enum Connection : uint8_t {
    SUCCESS,
    DISCONNECTED,
    ERROR
};

struct Socket {
private:
    SocketHandle sock = INVALID_SOCKET_HANDLE;
    std::mutex writeMutex;

    Socket(SocketHandle sock) : sock(sock) {
    }

    friend struct ServerSocket;

public:
    Socket(const char* host, uint32_t port);

    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other);
    Socket& operator=(Socket&& other);

    Connection read(void* data, uint32_t length);

    Connection write(const void* data, uint32_t length);

    std::string getIP() const;

    void close();
};

#endif