#include "Multiplayer.h"

#include <iostream>

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

void Multiplayer::init() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "Failed to initialize WinSock API" << std::endl;
    }
}

void Multiplayer::exit() {
    WSACleanup();
}

#else

void Multiplayer::init() {
}

void Multiplayer::exit() {
}

#endif