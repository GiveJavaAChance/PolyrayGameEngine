#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#pragma once

#include <atomic>
#include <cstring>
#include <thread>
#include <unordered_map>

#include <net/Multiplayer.h>

#include <net/Packet.h>
#include <net/PacketType.h>
#include <net/Socket.h>

namespace Multiplayer {
    struct PacketRegistry {
        DynamicArray<PacketType> packetTypes;
        std::unordered_map<std::string, uint32_t> packetTypeMap;

        void createPacketType(const char* tag, std::function<void(PacketWriter)> serializer, std::function<void(uint32_t, PacketReader)> deserializer) {
            uint32_t idx = packetTypes.size();
            packetTypes.emplace(serializer, deserializer);
            packetTypeMap[tag] = idx;
        }
    };

    struct Client {
    private:
        Socket socket;

        PacketRegistry packetRegistry;

        std::atomic<bool> running;
        uint32_t clientID;
        std::thread listenerThread;

        std::mutex packetQueueMutex;

        DynamicArray<Packet> packetQueue;
        DynamicArray<Packet> pollBuffer;

        DynamicArray<uint8_t> sendBuffer;

        void listen();

    public:
        Client(const char* host, uint32_t port);

        ~Client();

        void pollPackets();

        void send(const char* tag);
    };
}
#endif
