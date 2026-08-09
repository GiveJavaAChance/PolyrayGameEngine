#ifndef MULTIPLAYER_H_INCLUDED
#define MULTIPLAYER_H_INCLUDED

#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

#include <net/Socket.h>

struct PacketType {
    bool isDirect;
    void* serializer;
    void (*deserializer)(uint8_t*, uint32_t, uint32_t);

    PacketType(bool d, void* ser, void (*des)(uint8_t*, uint32_t, uint32_t)) : isDirect(d), serializer(ser), deserializer(des) {
    }
};

struct PacketData {
    uint8_t* data;
    uint32_t size;

    PacketData(uint32_t size) : data(new uint8_t[size + 12u] + 12u), size(size) {
    }
};

struct PacketRegistry {
    std::vector<PacketType> types;

    template <typename T>
    uint32_t createObjectPacketType(PacketData (*serializer)(const T&), void (*deserializer)(uint8_t*, uint32_t, uint32_t), bool direct = false) {
        uint32_t id = types.size();
        types.emplace_back(direct, (void*) serializer, deserializer);
        return id;
    }

    template <typename T, void (*Func)(T&, uint32_t)>
    uint32_t createDefaultObjectPacketType() {
        static_assert(std::is_trivially_copyable_v<T>);
        return createObjectPacketType<T>([](const T& obj) {
            PacketData data(sizeof(T));
            std::memcpy(data.data, &obj, sizeof(T));
            return data; }, [](uint8_t* data, uint32_t size, uint32_t clientID) {
            T obj;
            std::memcpy(&obj, data, sizeof(obj));
            Func(obj, clientID); });
    }

    uint32_t createStatelessPacketType(PacketData (*serializer)(), void (*deserializer)(uint8_t*, uint32_t, uint32_t), bool direct = false) {
        uint32_t id = types.size();
        types.emplace_back(direct, (void*) serializer, deserializer);
        return id;
    }

    inline PacketType& operator[](uint32_t idx) {
        return types[idx];
    }
};

struct Multiplayer {
private:
    struct PacketEntry {
        uint8_t* data;
        uint32_t size;
        uint32_t clientID;
        uint32_t packetID;
    };
    PacketRegistry packetRegistry;
    Socket socket;
    std::atomic<bool> running;
    uint32_t clientID;
    std::thread listenerThread;
    std::vector<PacketEntry> packetQueue;
    std::mutex packetQueueMutex;

    void listen() {
        uint32_t header[3];
        try {
            while (running) {
                socket.read((uint8_t*) &header, 12u);
                uint32_t clientID = ntohl(header[0]);
                uint32_t packetID = ntohl(header[1]);
                uint32_t size = header[2];
                const PacketType& type = packetRegistry[packetID];
                uint8_t* buffer = new uint8_t[size];
                socket.read(buffer, size);
                if (type.isDirect) {
                    type.deserializer(buffer, size, clientID);
                    delete[] buffer;
                } else {
                    std::lock_guard<std::mutex> lock(packetQueueMutex);
                    packetQueue.emplace_back(buffer, size, clientID, packetID);
                }
            }
        } catch (...) {
        }
    }

public:
    Multiplayer(const PacketRegistry& pr, const char* h, uint32_t p) : packetRegistry(pr), socket(h, p), running(true) {
        socket.read((uint8_t*) &clientID, 4u);
        clientID = ntohl(clientID);
        this->listenerThread = std::thread(&Multiplayer::listen, this);
    }

    ~Multiplayer() {
        running = false;
        socket.close();
        listenerThread.join();
    }

    void pollPackets() {
        std::vector<PacketEntry> packets;
        {
            std::lock_guard<std::mutex> lock(packetQueueMutex);
            packets.swap(packetQueue);
        }
        for (PacketEntry& packet : packets) {
            const PacketType& type = packetRegistry[packet.packetID];
            type.deserializer(packet.data, packet.size, packet.clientID);
            delete[] packet.data;
        }
    }

    void send(uint32_t type) {
        PacketData data = ((PacketData (*)()) packetRegistry[type].serializer)();

        uint32_t header[3];
        header[0] = htonl(clientID);
        header[1] = htonl(type);
        header[2] = data.size;

        data.data -= 12u;
        std::memcpy(data.data, header, 12u);
        socket.write(data.data, data.size + 12u);
        delete[] data.data;
    }

    template <typename T>
    void send(uint32_t type, const T& obj) {
        PacketData data = ((PacketData (*)(const T&)) packetRegistry[type].serializer)(obj);

        uint32_t header[3];
        header[0] = htonl(clientID);
        header[1] = htonl(type);
        header[2] = data.size;

        data.data -= 12u;
        std::memcpy(data.data, header, 12u);
        socket.write(data.data, data.size + 12u);
        delete[] data.data;
    }
};

#endif
