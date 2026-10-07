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
    Arena arena;
    ThreadPool<8u> threadPool;

    ECS ecs;
    EventBus eventBus;

    DynamicArray<void*> systems;

    World() : arena(), ecs(arena) {
    }

    World(const World&) = delete;
    World& operator=(const World&) = delete;

    World(World&& other) noexcept
        : arena(std::move(other.arena)), ecs(std::move(other.ecs)),
          eventBus(std::move(other.eventBus)), systems(std::move(other.systems)) {
    }

    World& operator=(World&& other) noexcept {
        if (this != &other) {
            arena = std::move(other.arena);
            ecs = std::move(other.ecs);
            eventBus = std::move(other.eventBus);
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
        systems[idx] = &arena.emplace<Sys>(std::forward<Args>(args)...);
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
