#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#pragma once

#include <cstdint>
#include <iostream>

#include <EventBus.h>
#include <ecs/ECS.h>
#include <structure/Arena.h>
#include <utils/ThreadPool.h>

struct World {
private:
    TYPE_REGISTRY(SystemTypes)

public:
    ECS ecs;
    EventBus eventBus;

    ThreadPool<8u> threadPool;

    Arena systemArena;
    DynamicArray<void*> systems;

    World() {
    }

    World(const World&) = delete;
    World& operator=(const World&) = delete;

    World(World&& other) noexcept
        : ecs(std::move(other.ecs)), eventBus(std::move(other.eventBus)),
          systemArena(std::move(other.systemArena)), systems(std::move(other.systems)) {
    }

    World& operator=(World&& other) noexcept {
        if (this != &other) {
            ecs = std::move(other.ecs);
            eventBus = std::move(other.eventBus);
            systemArena = std::move(other.systemArena);
            systems = std::move(other.systems);
        }
        return *this;
    }

    template <typename Sys, typename... Args>
    void createSystem(Args&&... args) {
        uint32_t idx = SystemTypes::getTypeId<Sys>();
        while (idx >= systems.size()) {
            systems.emplace(nullptr);
        }
        systems[idx] = &systemArena.emplace<Sys>(std::forward<Args>(args)...);
    }

    template <typename Sys>
    Sys* getSystem() {
        uint32_t id = SystemTypes::getTypeId<Sys>();
        if (id > systems.size()) {
            return nullptr;
        }
        return reinterpret_cast<Sys*>(systems[id]);
    }

    void update(double dt) {
        threadPool.start();
        ecs.update(dt);
        threadPool.stop();
    }
};

#endif
