#include "Client.h"

void Multiplayer::Client::listen() {
    uint32_t header[3u];
    while (running.load(std::memory_order_relaxed)) {
        if (socket.read(header, 12u) != Connection::SUCCESS) {
            return;
        }
        uint32_t packetSize = ntoh(header[0u]);
        if (packetSize < 12u || packetSize > MAX_PACKET_SIZE) {
            return;
        }
        uint32_t clientID = ntoh(header[1u]);
        uint32_t packetID = ntoh(header[2u]);
        uint8_t* buffer = alloc<uint8_t>(packetSize - 12u);
        if (socket.read(buffer, packetSize - 12u) != Connection::SUCCESS) {
            return;
        }
        {
            std::lock_guard<std::mutex> lock(packetQueueMutex);
            packetQueue.emplace(clientID, packetID, buffer);
        }
    }
}

Multiplayer::Client::Client(const char* host, uint32_t port) : socket(host, port), running(true) {
    socket.read((uint8_t*) &clientID, 4u);
    clientID = ntoh(clientID);
    this->listenerThread = std::thread(&Multiplayer::Client::listen, this);
}

Multiplayer::Client::~Client() {
    running.store(false, std::memory_order_relaxed);
    socket.close();
    listenerThread.join();
}

void Multiplayer::Client::pollPackets() {
    {
        std::lock_guard<std::mutex> lock(packetQueueMutex);
        std::swap(packetQueue, pollBuffer);
    }
    for (uint32_t i = 0u; i < pollBuffer.size(); i++) {
        Packet& packet = pollBuffer[i];
        const PacketType& type = packetRegistry.packetTypes[packet.packetID];
        type.deserializer(packet.clientID, PacketReader(packet.data));
        free((uint8_t*) packet.data);
    }
    pollBuffer.clear();
}

void Multiplayer::Client::send(const char* tag) {
    uint32_t type = packetRegistry.packetTypeMap[tag];
    sendBuffer.clear();
    uint32_t header[3u];
    header[0u] = 0u;
    header[1u] = hton(clientID);
    header[2u] = hton(type);
    sendBuffer.addAll(reinterpret_cast<uint8_t*>(header), sizeof(header));
    packetRegistry.packetTypes[type].serializer(PacketWriter(sendBuffer));

    uint32_t size = hton(sendBuffer.size());
    std::memcpy(sendBuffer.data(), &size, sizeof(uint32_t));

    if (socket.write(sendBuffer.data(), sendBuffer.size()) != Connection::SUCCESS) {
        // error
    }
}