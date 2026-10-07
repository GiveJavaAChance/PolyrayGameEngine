#include "Socket.h"
#include "SocketHandle.h"

#include <cstdint>
#include <stdexcept>

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#else

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#endif

#undef ERROR

inline void closeSocket(SocketHandle socket) {
#ifdef _WIN32
    closesocket(socket);
#else
    close(socket);
#endif
}

Socket::Socket(const char* host, uint32_t port) {
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET_HANDLE) {
        throw std::runtime_error("Socket creation failed");
    }
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, host, &server.sin_addr) <= 0) {
        close();
        throw std::runtime_error("Invalid address");
    }
    if (connect(sock, reinterpret_cast<sockaddr*>(&server), sizeof(server)) < 0) {
        close();
        throw std::runtime_error("Connection failed");
    }
}

Socket::~Socket() {
    close();
}

Socket::Socket(Socket&& other) : sock(other.sock) {
    other.sock = INVALID_SOCKET_HANDLE;
}

Socket& Socket::operator=(Socket&& other) {
    if (this != &other) {
        sock = other.sock;
        other.sock = INVALID_SOCKET_HANDLE;
    }
    return *this;
}

Connection Socket::read(void* data, uint32_t length) {
    uint32_t total = 0;
    while (total < length) {
        int bytes = recv(sock, reinterpret_cast<char*>(data) + total, length - total, 0);
        if (bytes == 0) {
            return Connection::DISCONNECTED;
        } else if (bytes == -1) {
            return Connection::ERROR;
        }
        total += static_cast<uint32_t>(bytes);
    }
    return Connection::SUCCESS;
}

Connection Socket::write(const void* data, uint32_t length) {
    std::lock_guard<std::mutex> lock(writeMutex);
    uint32_t total = 0;
    while (total < length) {
        int bytes = send(sock, reinterpret_cast<const char*>(data) + total, length - total, 0);
        if (bytes == -1) {
            return Connection::ERROR;
        }
        total += static_cast<uint32_t>(bytes);
    }
    return Connection::SUCCESS;
}

std::string Socket::getIP() const {
    sockaddr_in address{};
#ifdef _WIN32
    int addressLength = sizeof(sockaddr_in);
#else
    socklen_t addressLength = sizeof(sockaddr_in);
#endif
    if (getpeername(sock, reinterpret_cast<sockaddr*>(&address), &addressLength) != 0) {
        return {};
    }
    char ip[INET_ADDRSTRLEN]{};
    if (inet_ntop(AF_INET, &address.sin_addr, ip, sizeof(ip)) == nullptr) {
        return {};
    }
    return ip;
}

void Socket::close() {
    if (sock != INVALID_SOCKET_HANDLE) {
        closeSocket(sock);
        sock = INVALID_SOCKET_HANDLE;
    }
}