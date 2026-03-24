#ifndef ECS_H_INCLUDED
#define ECS_H_INCLUDED

#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>

#include "typereg.h"
#include "DynamicArray.h"
#include "MultiDynamicArray.h"
#include "Registry.h"
#include "EventBus.h"

struct ECS;

template<typename T>
concept Component = std::is_trivially_copyable_v<T>
                 && std::is_trivially_destructible_v<T>;
                 //&& std::is_standard_layout_v<T>;

template<Component T>
struct ComponentRef {
    uint32_t ID;
};

template<Component T>
struct ComponentView {
private:
    ECS* ecs;
    ComponentRef<T> ref;
    T temp;
    bool dirty = false;

public:
    ComponentView(ECS* ecs, ComponentRef<T> ref);

    ~ComponentView();

    inline T* operator->() {
        dirty = true;
        return &temp;
    }

    inline T& operator*()  {
        dirty = true;
        return temp;
    }
};

enum Layout : uint8_t {
    AoS,
    SoA
};

enum UpdateOrder : uint8_t {
    PRE_PHYSICS,
    PHYSICS,
    POST_PHYSICS,
    FRAME,
    PRE_RENDER,
    RENDERING
};

template<Component T, Layout L = Layout::AoS>
struct Storage;

template<typename T>
struct Storage<T, Layout::AoS> {
    DynamicArray<T> data;
    Registry reg;

    inline uint32_t add(const T& component) noexcept {
        data.add(component);
        return reg.create();
    }

    inline void set(uint32_t componentID, const T& component) noexcept {
        data[reg[componentID]] = component;
    }

    inline void remove(uint32_t componentID) noexcept {
        reg.remove(componentID, data);
        data.removeEnd(1u);
    }

    inline T& get(uint32_t id) const noexcept {
        return data[reg[id]];
    }
};

struct Entity {
    uint32_t entityID;

    constexpr explicit Entity(uint32_t id, ECS* ecs) : entityID(id), ecs(ecs) {
    }

    template<Component T>
    inline void addComponent(const T& component);

    template<Component T>
    inline bool setComponent(const T& component);

    template<Component T>
    inline void removeComponent();

    template<Component T>
    inline bool getComponent(T& out);

    template<Component T>
    inline T* getComponentPtr();

    template<Component T>
    inline uint32_t getComponentCount();

    template<Component T>
    inline uint32_t getComponents(T* const ptr);

private:
    ECS* ecs;
};

struct ECS {
private:
    constexpr static uint32_t TYPE_SHIFT = 48u;
    constexpr static uint64_t TYPE_MASK  = 0xFFFF000000000000ull;
    constexpr static uint64_t ID_MASK    = 0x0000FFFFFFFFFFFFull;

    DynamicArray<void*> storages;

    template<typename... Ts>
    struct Invoke {
        template<typename System, void (System::*func)(Ts...)>
        static void thunk(void* instance, Ts... args) {
            (reinterpret_cast<System*>(instance)->*func)(args...);
        }
    };

    struct ComponentListener {
        void* instance;
        void(*onAdded)(void*, Entity, uint32_t);
        void(*onRemoved)(void*, Entity, uint32_t);
    };

    template<typename... Ts>
    struct SystemCallback {
        void* instance;
        void(*func)(void*, Ts...);
    };

    DynamicArray<DynamicArray<ComponentListener>> listeners;

    DynamicArray<SystemCallback<>> setupCallbacks;

    DynamicArray<SystemCallback<double>> prePhysicsUpdateCallbacks;
    DynamicArray<SystemCallback<double>> physicsUpdateCallbacks;
    DynamicArray<SystemCallback<double>> postPhysicsUpdateCallbacks;
    DynamicArray<SystemCallback<double>> frameUpdateCallbacks;
    DynamicArray<SystemCallback<>> renderInitUpdateCallbacks;
    DynamicArray<SystemCallback<>> renderUpdateCallbacks;

    double fixedDT = 0.006d;
    double remaining = 0.0d;

    uint64_t** entities;

    uint32_t* componentCount;
    uint32_t* componentCapacity;

    uint32_t entityCount = 0u;
    uint32_t entityCapacity = 0u;

    void ensureEntityCapacity(uint32_t cap) {
        if(cap <= entityCapacity) {
            return;
        }
        cap = (cap * 3u) >> 1u;
        void* ptr = _mm_malloc(cap * (sizeof(uint64_t*) + 2u * sizeof(uint32_t)), 32u);
        uint64_t** newEntities = reinterpret_cast<uint64_t**>(ptr);
        uint32_t* newComponentCount = reinterpret_cast<uint32_t*>(newEntities + cap);
        uint32_t* newComponentCapacity = newComponentCount + cap;
        if(entities) {
            std::memcpy(newEntities, entities, entityCapacity * sizeof(uint64_t*));
            std::memcpy(newComponentCount, componentCount, entityCapacity * sizeof(uint32_t));
            std::memcpy(newComponentCapacity, componentCapacity, entityCapacity * sizeof(uint32_t));
            _mm_free(entities);
        }
        entities = newEntities;
        componentCount = newComponentCount;
        componentCapacity = newComponentCapacity;

        std::memset(componentCount + entityCapacity, 0, (cap - entityCapacity) * sizeof(uint32_t));

        entityCapacity = cap;
    }

    void ensureComponentCapacity(const uint32_t entityID, uint32_t cap) {
        if(cap <= componentCapacity[entityID]) {
            return;
        }
        cap = (cap * 3u) >> 1u;
        uint64_t* newComponents = reinterpret_cast<uint64_t*>(_mm_malloc(cap * sizeof(uint64_t), 32u));
        std::memcpy(newComponents, entities[entityID], componentCapacity[entityID] * sizeof(uint64_t));
        _mm_free(entities[entityID]);
        entities[entityID] = newComponents;
        componentCapacity[entityID] = cap;
    }

    template<Component T>
    Storage<T, Layout::AoS>* getStorage() {
        return reinterpret_cast<Storage<T, Layout::AoS>*>(storages[ComponentTypes::getTypeId<T>()]);
    }

    template<Component T>
    void componentAdded(const Entity& e, uint32_t id) {
        DynamicArray<ComponentListener>& list = listeners[ComponentTypes::getTypeId<T>()];
        for(uint32_t i = 0u; i < list.size(); i++) {
            ComponentListener& l = list[i];
            if(l.onAdded) {
                l.onAdded(l.instance, e, id);
            }
        }
    }

    template<Component T>
    void componentRemoved(const Entity& e, uint32_t id) {
        DynamicArray<ComponentListener>& list = listeners[ComponentTypes::getTypeId<T>()];
        for(uint32_t i = 0u; i < list.size(); i++) {
            ComponentListener& l = list[i];
            if(l.onRemoved) {
                l.onRemoved(l.instance, e, id);
            }
        }
    }

    template<typename... Ts>
    void invokeAll(DynamicArray<SystemCallback<Ts...>>& callbacks, Ts... args) {
        for(uint32_t i = 0u; i < callbacks.size(); i++) {
            SystemCallback<Ts...>& c = callbacks[i];
            c.func(c.instance, args...);
        }
    }

    TYPE_REGISTRY(ComponentTypes)
public:
    template<Component T, Layout L = Layout::AoS>
    void registerComponentType() {
        uint32_t idx = ComponentTypes::getTypeId<T>();
        while(idx >= storages.size()) {
            storages.add((void*)0);
        }
        while(idx >= listeners.size()) {
            listeners.emplace(1u);
        }
        if(!storages[idx]) {
            storages[idx] = new Storage<T, L>{};
        }
    }

    template<Component T, typename System, void (System::*onAdded)(Entity, uint32_t), void (System::*onRemoved)(Entity, uint32_t)>
    void registerComponentListener(System* system) {
        listeners[ComponentTypes::getTypeId<T>()].emplace(system, &Invoke<Entity, uint32_t>::thunk<System, onAdded>, &Invoke<Entity, uint32_t>::thunk<System, onRemoved>);
    }

    template<typename System, void(System::*setup)()>
    void registerSetupCallback(System* system) {
        setupCallbacks.emplace(system, &Invoke<>::thunk<System, setup>);
    }

    template<typename System, void(System::*update)(double), UpdateOrder Order>
    void registerUpdateCallback(System* system) {
        if constexpr (Order == UpdateOrder::PRE_PHYSICS) {
            prePhysicsUpdateCallbacks.emplace(system, &Invoke<double>::thunk<System, update>);
        } else if constexpr (Order == UpdateOrder::PHYSICS) {
            physicsUpdateCallbacks.emplace(system, &Invoke<double>::thunk<System, update>);
        } else if constexpr (Order == UpdateOrder::POST_PHYSICS) {
            postPhysicsUpdateCallbacks.emplace(system, &Invoke<double>::thunk<System, update>);
        } else if constexpr (Order == UpdateOrder::FRAME) {
            frameUpdateCallbacks.emplace(system, &Invoke<double>::thunk<System, update>);
        }
    }

    template<typename System, void(System::*update)(), UpdateOrder Order>
    void registerUpdateCallback(System* system) {
        if constexpr (Order == UpdateOrder::PRE_RENDER) {
            renderInitUpdateCallbacks.emplace(system, &Invoke<>::thunk<System, update>);
        } else {
            renderUpdateCallbacks.emplace(system, &Invoke<>::thunk<System, update>);
        }
    }

    Entity createEntity(const uint32_t initialCapacity = 8u) {
        const uint32_t id = entityCount++;
        ensureEntityCapacity(entityCount);
        entities[id] = reinterpret_cast<uint64_t*>(_mm_malloc(initialCapacity * sizeof(uint64_t), 32u));
        componentCapacity[id] = initialCapacity;
        return Entity(id, this);
    }

    template<Component T>
    void addComponent(const Entity& e, const T& component) {
        static uint32_t type = ComponentTypes::getTypeId<T>();
        static uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
        const uint32_t idx = componentCount[e.entityID]++;
        ensureComponentCapacity(e.entityID, componentCount[e.entityID]);
        const uint32_t componentID = getStorage<T>()->add(component);
        entities[e.entityID][idx] = componentType | static_cast<uint64_t>(componentID);
        componentAdded<T>(e, componentID);
    }

    template<Component T>
    bool setComponent(const Entity& e, const T& component) {
        static uint32_t type = ComponentTypes::getTypeId<T>();
        static uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
        Storage<T, Layout::AoS>* storage = getStorage<T>();
        const uint64_t* entity = entities[e.entityID];
        const uint32_t size = componentCount[e.entityID];
        for(uint32_t i = 0u; i < size; i++) {
            if((entity[i] & TYPE_MASK) == componentType) {
                storage->set(static_cast<uint32_t>(entity[i] & ID_MASK), component);
                return true;
            }
        }
        return false;
    }

    template<Component T>
    void removeComponent(const Entity& e) {
        static uint32_t type = ComponentTypes::getTypeId<T>();
        static uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
        Storage<T, Layout::AoS>* storage = getStorage<T>();
        uint64_t* entity = entities[e.entityID];
        const uint32_t size = componentCount[e.entityID];
        for(uint32_t i = 0u; i < size; i++) {
            if((entity[i] & TYPE_MASK) == componentType) {
                uint32_t componentID = static_cast<uint32_t>(entity[i] & ID_MASK);
                componentRemoved<T>(e, componentID);
                storage->remove(componentID);
                entity[i] = entity[--componentCount[e.entityID]];
                break;
            }
        }
    }

    template<Component T>
    bool getComponent(const Entity& e, T& out) {
        static uint32_t type = ComponentTypes::getTypeId<T>();
        static uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
        Storage<T, Layout::AoS>* storage = getStorage<T>();
        const uint64_t* entity = entities[e.entityID];
        const uint32_t size = componentCount[e.entityID];
        for(uint32_t i = 0u; i < size; i++) {
            if((entity[i] & TYPE_MASK) == componentType) {
                out = storage->get(static_cast<uint32_t>(entity[i] & ID_MASK));
                return true;
            }
        }
        return false;
    }

    template<Component T>
    T* getComponentPtr(const Entity& e) {
        static uint32_t type = ComponentTypes::getTypeId<T>();
        static uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
        Storage<T, Layout::AoS>* storage = getStorage<T>();
        const uint64_t* entity = entities[e.entityID];
        const uint32_t size = componentCount[e.entityID];
        for(uint32_t i = 0u; i < size; i++) {
            if((entity[i] & TYPE_MASK) == componentType) {
                return &storage->get(static_cast<uint32_t>(entity[i] & ID_MASK));
            }
        }
        return nullptr;
    }

    template<Component T>
    uint32_t getComponentCount(const Entity& e) {
        static uint64_t componentType = static_cast<uint64_t>(ComponentTypes::getTypeId<T>()) << TYPE_SHIFT;
        const uint64_t* entity = entities[e.entityID];
        const uint32_t size = componentCount[e.entityID];
        uint32_t count = 0u;
        for(uint32_t i = 0u; i < size; i++) {
            if((entity[i] & TYPE_MASK) == componentType) {
                count++;
            }
        }
        return count;
    }

    template<Component T>
    uint32_t getComponents(const Entity& e, T* const ptr) {
        static uint32_t type = ComponentTypes::getTypeId<T>();
        static uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
        Storage<T, Layout::AoS>*& storage = getStorage<T>();
        const uint64_t* entity = entities[e.entityID];
        const uint32_t size = componentCount[e.entityID];
        uint32_t idx = 0u;
        for(uint32_t i = 0u; i < size; i++) {
            if((entity[i] & TYPE_MASK) == componentType) {
                ptr[idx++] = storage->get(static_cast<uint32_t>(entity[i] & ID_MASK));
            }
        }
        return idx;
    }

    template<Component T>
    Storage<T, Layout::AoS>& view() {
        return *getStorage<T>();
    }

    template<Component T>
    T* getPtr(ComponentRef<T> ref) {
        return &getStorage<T>()->get(ref.ID);
    }

    template<Component T>
    T read(ComponentRef<T> ref) {
        return getStorage<T>()->get(ref.ID);
    }

    template<Component T>
    void write(ComponentRef<T> ref, const T& component) {
        getStorage<T>()->set(ref.ID, component);
    }

    template<Component T>
    bool getComponentID(const Entity& e, uint32_t& id) {
        static uint32_t type = ComponentTypes::getTypeId<T>();
        static uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
        const uint64_t* entity = entities[e.entityID];
        const uint32_t size = componentCount[e.entityID];
        for(uint32_t i = 0u; i < size; i++) {
            if((entity[i] & TYPE_MASK) == componentType) {
                id = static_cast<uint32_t>(entity[i] & ID_MASK);
                return true;
            }
        }
        return false;
    }

    void setup() {
        invokeAll<>(setupCallbacks);
    }

    void update(double dt) {
        invokeAll<double>(prePhysicsUpdateCallbacks, dt);
        remaining += dt;
        while(remaining >= fixedDT) {
            invokeAll<double>(physicsUpdateCallbacks, fixedDT);
            remaining -= fixedDT;
        }
        invokeAll<double>(postPhysicsUpdateCallbacks, dt);
        invokeAll<double>(frameUpdateCallbacks, dt);
        invokeAll<>(renderInitUpdateCallbacks);
        invokeAll<>(renderUpdateCallbacks);
    }

    void setPhysicsDT(double dt) {
        fixedDT = dt;
    }
};

template<Component T>
ComponentView<T>::ComponentView(ECS* ecs, ComponentRef<T> ref) : ecs(ecs), ref(ref), temp(ecs->read(ref)) {
}

template<Component T>
ComponentView<T>::~ComponentView() {
    if (dirty) {
        ecs->write(ref, temp);
    }
}

template<Component T>
inline void Entity::addComponent(const T& component) {
    return ecs->addComponent<T>(*this, component);
}

template<Component T>
inline bool Entity::setComponent(const T& component) {
    return ecs->setComponent<T>(*this, component);
}

template<Component T>
inline void Entity::removeComponent() {
    return ecs->removeComponent<T>(*this);
}

template<Component T>
inline bool Entity::getComponent(T& out) {
    return ecs->getComponent<T>(*this, out);
}

template<Component T>
inline T* Entity::getComponentPtr() {
    return ecs->getComponentPtr<T>(*this);
}

template<Component T>
inline uint32_t Entity::getComponentCount() {
    return ecs->getComponentCount<T>(*this);
}

template<Component T>
inline uint32_t Entity::getComponents(T* const ptr) {
    return ecs->getComponents<T>(*this, ptr);
}

#endif
