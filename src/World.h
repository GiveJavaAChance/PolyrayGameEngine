#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#pragma once

#include <cstdint>
#include <iostream>

#include <EventBus.h>
#include <ecs/ECS.h>
#include <utils/ThreadPool.h>

struct World {
private:
    TYPE_REGISTRY(SystemTypes)

    struct System {
        void* instance;
        void (*destruct)(void*);
    };

public:
    ECS ecs;
    EventBus eventBus;

    ThreadPool<8u> threadPool;

    DynamicArray<System> systems;

    World() {
    }

    ~World() {
        for (uint32_t i = 0u; i < systems.size(); i++) {
            System& system = systems[i];
            if (system.instance && system.destruct) {
                system.destruct(system.instance);
            }
        }
    }

    World(const World&) = delete;
    World& operator=(const World&) = delete;

    World(World&& other) noexcept : ecs(std::move(other.ecs)), eventBus(std::move(other.eventBus)), systems(std::move(other.systems)) {
    }

    World& operator=(World&& other) noexcept {
        if (this != &other) {
            ecs = std::move(other.ecs);
            eventBus = std::move(other.eventBus);
            systems = std::move(other.systems);
        }
        return *this;
    }

    template <typename Sys>
    void addSystem(Sys* system) {
        uint32_t idx = SystemTypes::getTypeId<Sys>();
        while (idx >= systems.size()) {
            systems.emplace(nullptr, nullptr);
        }
        systems[idx] = System{system, [](void* ptr) { delete reinterpret_cast<Sys*>(ptr); }};
    }

    template <typename Sys>
    Sys* getSystem() {
        uint32_t id = SystemTypes::getTypeId<Sys>();
        if (id > systems.size()) {
            return nullptr;
        }
        return reinterpret_cast<Sys*>(systems[id].instance);
    }

    void update(double dt) {
        threadPool.start();
        ecs.update(dt);
        threadPool.stop();
    }
};

#endif
