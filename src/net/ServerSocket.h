#ifndef SERVERSOCKET_H_INCLUDED
#define SERVERSOCKET_H_INCLUDED

#pragma once

#include <net/Socket.h>

struct ServerSocket {
private:
    SocketHandle sock = INVALID_SOCKET_HANDLE;

public:
    ServerSocket(uint32_t port);

    ~ServerSocket();

    ServerSocket(const ServerSocket&) = delete;
    ServerSocket& operator=(const ServerSocket&) = delete;

    ServerSocket(ServerSocket&& other);
    ServerSocket& operator=(ServerSocket&& other);

    Socket accept();

    void close();
};

#endif
