#include "Server.h"

#include <iostream>
#include <thread>

#include <utils/net_order.h>

#include <net/Packet.h>

void Multiplayer::Server::broadcast(const void* packetData, uint32_t packetSize, uint32_t senderID) {
    DynamicArray<Socket*> targets;
    {
        std::shared_lock<std::shared_mutex> lock(clientsMutex);
        targets.ensureCapacity(clients.size() - 1u);
        for (uint32_t clientID : clients.reg) {
            if (clientID != senderID) {
                targets.add(clients[clientID]);
            }
        }
    }
    for (uint32_t i = 0u; i < targets.size(); i++) {
        targets[i]->write(packetData, packetSize);
    }
}

void Multiplayer::Server::disconnect(uint32_t clientID) {
    Socket* client = nullptr;
    {
        std::shared_lock<std::shared_mutex> lock(clientsMutex);
        client = clients[clientID];
    }
    if (!client) {
        return;
    }
    client->close();
    delete client;
    {
        std::unique_lock<std::shared_mutex> lock(clientsMutex);
        clients.remove(clientID);
    }
    {
        std::lock_guard<std::mutex> lock(logMutex);
        std::cout << "Client " << clientID << " disconnected.\n";
    }
}

void Multiplayer::Server::handleClient(uint32_t clientID) {
    alignas(4u) uint8_t packetDataBuffer[MAX_PACKET_SIZE];
    while (true) {
        Socket* client = nullptr;
        {
            std::shared_lock<std::shared_mutex> lock(clientsMutex);
            client = clients[clientID];
        }
        if (client->read(packetDataBuffer, 4u) != Connection::SUCCESS) {
            disconnect(clientID);
            return;
        }
        uint32_t packetSize = ntoh(*reinterpret_cast<const uint32_t*>(packetDataBuffer));
        if (packetSize < 4u || packetSize > MAX_PACKET_SIZE) {
            {
                std::lock_guard<std::mutex> lock(logMutex);
                std::cout << "Client " << clientID << " sent a broken packet. Packet size: " << packetSize << std::endl;
            }
            disconnect(clientID);
            return;
        }
        if (client->read(packetDataBuffer + 4u, packetSize - 4u) != Connection::SUCCESS) {
            disconnect(clientID);
            return;
        }
        broadcast(packetDataBuffer, packetSize, clientID);
    }
}

Multiplayer::Server::Server(uint32_t port) : server(port) {
    std::cout << "Started server." << std::endl;
    while (true) {
        Socket client = server.accept();
        Socket* heapSocket = new Socket(std::move(client));
        uint32_t clientID = UINT32_MAX;
        {
            std::unique_lock<std::shared_mutex> lock(clientsMutex);
            clientID = clients.emplace(heapSocket);
        }
        {
            std::lock_guard<std::mutex> lock(logMutex);
            std::cout << "Client " << heapSocket->getIP() << " with ID: " << clientID << " connected." << std::endl;
        }
        uint32_t id = hton(clientID);
        client.write(&id, 4u);
        std::thread(&Multiplayer::Server::handleClient, this, clientID).detach();
    }
}