#include "ServerSocket.h"
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

ServerSocket::ServerSocket(uint32_t port) {
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET_HANDLE) {
        throw std::runtime_error("Socket creation failed");
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(static_cast<uint16_t>(port));
    if (bind(sock, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        close();
        throw std::runtime_error("Socket bind failed");
    }
    if (listen(sock, SOMAXCONN) < 0) {
        close();
        throw std::runtime_error("Socket listen failed");
    }
}

ServerSocket::~ServerSocket() {
    close();
}

ServerSocket::ServerSocket(ServerSocket&& other) : sock(other.sock) {
    other.sock = INVALID_SOCKET_HANDLE;
}

ServerSocket& ServerSocket::operator=(ServerSocket&& other) {
    if (this != &other) {
        sock = other.sock;
        other.sock = INVALID_SOCKET_HANDLE;
    }
    return *this;
}

Socket ServerSocket::accept() {
    SocketHandle client = ::accept(sock, nullptr, nullptr);
    if (client == INVALID_SOCKET_HANDLE) {
        throw std::runtime_error("Socket accept failed");
    }
    return Socket(client);
}

void ServerSocket::close() {
    if (sock != INVALID_SOCKET_HANDLE) {
        closeSocket(sock);
        sock = INVALID_SOCKET_HANDLE;
    }
}