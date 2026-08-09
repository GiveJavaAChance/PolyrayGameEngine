#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#pragma once

#include <cstdint>
#include <stdexcept>

// I'll add cross-platform support later
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#endif

struct Socket {
private:
    SOCKET sock = INVALID_SOCKET;

public:
    Socket(const char* host, uint32_t port) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            throw std::runtime_error("Socket creation failed");
        }
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            throw std::runtime_error("Socket creation failed");
        }
        sockaddr_in server{};
        server.sin_family = AF_INET;
        server.sin_port = htons((u_short) port);
        if (inet_pton(AF_INET, host, &server.sin_addr) <= 0) {
            throw std::runtime_error("Invalid address");
        }
        if (connect(sock, (sockaddr*) &server, sizeof(server)) == SOCKET_ERROR) {
            closesocket(sock);
            WSACleanup();
            throw std::runtime_error("Connection failed");
        }
    }

    void read(uint8_t* data, uint32_t length) {
        uint32_t total = 0;
        while (total < length) {
            int bytes = recv(sock, (char*) (data + total), length - total, 0);
            if (bytes <= 0) {
                throw std::runtime_error("Disconnected");
            }
            total += bytes;
        }
    }

    void write(uint8_t* data, uint32_t length) {
        uint32_t total = 0;
        while (total < length) {
            int bytes = send(sock, (const char*) (data + total), length - total, 0);
            if (bytes <= 0) {
                throw std::runtime_error("Disconnected");
            }
            total += bytes;
        }
    }

    void close() {
        if (sock != INVALID_SOCKET) {
            closesocket(sock);
            sock = INVALID_SOCKET;
        }
    }

    ~Socket() {
        close();
        WSACleanup();
    }
};

#endif
