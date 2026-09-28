#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#pragma once

#include <shared_mutex>

#include <structure/UnorderedRegistry.h>

#include <net/Multiplayer.h>

#include <net/ServerSocket.h>

namespace Multiplayer {
    struct Server {
    private:
        ServerSocket server;

        std::shared_mutex clientsMutex;
        UnorderedRegistry<Socket*> clients;

        std::mutex logMutex;

        void broadcast(const void* packetData, uint32_t packetSize, uint32_t senderID);

        void disconnect(uint32_t clientID);

        void handleClient(uint32_t clientID);

    public:
        Server(uint32_t port);
    };
}

#endif