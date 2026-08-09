#ifndef ECS_H_INCLUDED
#define ECS_H_INCLUDED

#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>

#include <InvokeUtils.h>
#include <structure/DynamicArray.h>
#include <structure/MultiDynamicArray.h>
#include <structure/Registry.h>
#include <typereg.h>

#include <ecs/Component.h>
#include <ecs/ComponentMetadata.h>
#include <ecs/ComponentRef.h>
#include <ecs/Storage.h>

#include <utils/perf.h>

struct ECS;

struct Entity {
    uint32_t entityID;
    ECS* ecs;

    template <Component T>
    inline void addComponent(const T& component);

    template <Component T>
    inline bool setComponent(const T& component);

    template <Component T>
    inline void removeComponent();

    template <Component T>
    inline bool getComponent(T& out);

    template <Component T>
    inline T* getComponentPtr();

    template <Component T>
    inline uint32_t getComponentCount();

    template <Component T>
    inline uint32_t getComponents(T* const ptr);
};

template <Component T>
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

    inline T& operator*() {
        dirty = true;
        return temp;
    }
};

struct ComponentReflection {
    void* (*allocateStorage)();
    void (*freeStorage)(void*);
    void (*createAndAddComponent)(void*, uint32_t);
    void (*removeComponent)(void*, uint32_t);
    void (*removeComponentByID)(void*, uint32_t, uint32_t);
    void* (*getComponentPtr)(void*, uint32_t);
    bool (*isComponentValid)(void*, uint32_t);
    void* (*getPtr)(void*, uint32_t);
    void (*read)(void*, uint32_t, void*);
    void (*write)(void*, uint32_t, void*);
};

struct ComponentRegistry {
private:
    template <Component T>
    inline static T defaultConstruct() {
        return T{};
    }

public:
    inline static DynamicArray<ComponentMetadata> metadata;
    inline static DynamicArray<ComponentReflection> reflection;

    template <Component T, T (*Create)() = defaultConstruct<T>>
    inline static void registerComponentType();

    inline static void setStorageAllocator(uint32_t typeId, void* (*allocateStorage)(), void (*freeStorage)(void*)) {
        reflection[typeId].allocateStorage = allocateStorage;
        reflection[typeId].freeStorage = freeStorage;
    }
};

enum UpdateOrder : uint8_t {
    PRE_PHYSICS,
    PHYSICS,
    POST_PHYSICS,
    FRAME,
    POST_FRAME,
};

struct ECS {
private:
    constexpr static uint32_t TYPE_SHIFT = 48u;
    constexpr static uint64_t TYPE_MASK = 0xFFFF000000000000ull;
    constexpr static uint64_t ID_MASK = 0x0000FFFFFFFFFFFFull;

    template <Component T>
    inline static T defaultConstruct() {
        return T{};
    }

    struct ComponentListener {
        void* instance;
        void (*onAdded)(void*, Entity, uint32_t);
        void (*onRemoved)(void*, Entity, uint32_t);
    };

    template <typename... Ts>
    struct SystemCallback {
        void* instance;
        void (*func)(void*, Ts...);
    };

    DynamicArray<void*> storages;

    DynamicArray<DynamicArray<ComponentListener>> listeners;

    DynamicArray<SystemCallback<>> setupCallbacks;

    DynamicArray<SystemCallback<double>> prePhysicsUpdateCallbacks;
    DynamicArray<SystemCallback<double>> physicsUpdateCallbacks;
    DynamicArray<SystemCallback<double>> postPhysicsUpdateCallbacks;
    DynamicArray<SystemCallback<double>> frameUpdateCallbacks;
    DynamicArray<SystemCallback<double>> postFrameUpdateCallbacks;

    Registry entityRegistry;

    double fixedDT;
    double remaining;

    uint64_t** entities;
    uint32_t* componentCount;
    uint32_t* componentCapacity;

    uint32_t entityCount;
    uint32_t entityCapacity;

    void ensureEntityCapacity(uint32_t cap) {
        if (cap <= entityCapacity) {
            return;
        }
        cap = (cap * 3u) >> 1u;
        void* ptr = alloc<uint8_t>(cap * (sizeof(uint64_t*) + 2u * sizeof(uint32_t)));
        uint64_t** newEntities = reinterpret_cast<uint64_t**>(ptr);
        uint32_t* newComponentCount = reinterpret_cast<uint32_t*>(newEntities + cap);
        uint32_t* newComponentCapacity = newComponentCount + cap;
        if (entities) {
            std::memcpy(newEntities, entities, entityCapacity * sizeof(uint64_t*));
            std::memcpy(newComponentCount, componentCount, entityCapacity * sizeof(uint32_t));
            std::memcpy(newComponentCapacity, componentCapacity, entityCapacity * sizeof(uint32_t));
            free(entities);
        }
        entities = newEntities;
        componentCount = newComponentCount;
        componentCapacity = newComponentCapacity;

        std::memset(componentCount + entityCapacity, 0, (cap - entityCapacity) * sizeof(uint32_t));

        entityCapacity = cap;
    }

    void ensureComponentCapacity(uint32_t entityLocation, uint32_t cap) {
        if (cap <= componentCapacity[entityLocation]) {
            return;
        }
        cap = (cap * 3u) >> 1u;
        uint64_t* newComponents = alloc<uint64_t>(cap);
        std::memcpy(newComponents, entities[entityLocation], componentCapacity[entityLocation] * sizeof(uint64_t));
        free(entities[entityLocation]);
        entities[entityLocation] = newComponents;
        componentCapacity[entityLocation] = cap;
    }

    template <Component T>
    Storage<T>* getStorage() {
        return reinterpret_cast<Storage<T>*>(storages[ComponentMetadata::typeOf<T>()]);
    }

    void componentAdded(uint32_t entityID, uint32_t type, uint32_t componentID) {
        Entity e{entityID, this};
        DynamicArray<ComponentListener>& list = listeners[type];
        for (uint32_t i = 0u; i < list.size(); i++) {
            ComponentListener& l = list[i];
            if (l.onAdded) {
                l.onAdded(l.instance, e, componentID);
            }
        }
    }

    void componentRemoved(uint32_t entityID, uint32_t type, uint32_t componentID) {
        Entity e{entityID, this};
        DynamicArray<ComponentListener>& list = listeners[type];
        for (uint32_t i = 0u; i < list.size(); i++) {
            ComponentListener& l = list[i];
            if (l.onRemoved) {
                l.onRemoved(l.instance, e, componentID);
            }
        }
    }

    template <typename... Ts>
    void invokeAll(DynamicArray<SystemCallback<Ts...>>& callbacks, Ts... args) {
        for (uint32_t i = 0u; i < callbacks.size(); i++) {
            SystemCallback<Ts...>& c = callbacks[i];
            c.func(c.instance, args...);
        }
    }

    template <Component T>
    void read(uint32_t componentID, void* dst) {
        T component = read<T>(componentID);
        std::memcpy(dst, &component, sizeof(T));
    }

    template <Component T>
    void write(uint32_t componentID, void* src) {
        alignas(T) uint8_t component[sizeof(T)];
        std::memcpy(component, src, sizeof(T));
        write<T>(componentID, *reinterpret_cast<T*>(component));
    }

public:
    ECS() : fixedDT(0.006), remaining(0.0), entities(nullptr), componentCount(nullptr), componentCapacity(nullptr), entityCount(0u), entityCapacity(0u) {
        uint32_t size = ComponentRegistry::metadata.size();
        storages.ensureCapacity(size);
        listeners.ensureCapacity(size);
        for (uint32_t i = 0u; i < size; i++) {
            const ComponentMetadata& metadata = ComponentRegistry::metadata[i];
            if (metadata.typeId == UINT32_MAX) {
                continue;
            }
            const ComponentReflection& reflection = ComponentRegistry::reflection[i];
            storages.add(reflection.allocateStorage());
            listeners.emplace(1u);
        }
    }

    ~ECS() {
        if (entities) {
            free(entities);
        }
        for (uint32_t i = 0u; i < storages.size(); i++) {
            const ComponentReflection& reflection = ComponentRegistry::reflection[i];
            reflection.freeStorage(storages[i]);
        }
    }

    ECS(const ECS&) = delete;
    ECS& operator=(const ECS&) = delete;

    ECS(ECS&& other) noexcept
        : storages(std::move(other.storages)),
          listeners(std::move(other.listeners)),
          setupCallbacks(std::move(other.setupCallbacks)),
          prePhysicsUpdateCallbacks(std::move(other.prePhysicsUpdateCallbacks)),
          physicsUpdateCallbacks(std::move(other.physicsUpdateCallbacks)),
          postPhysicsUpdateCallbacks(std::move(other.postPhysicsUpdateCallbacks)),
          frameUpdateCallbacks(std::move(other.frameUpdateCallbacks)),
          postFrameUpdateCallbacks(std::move(other.postFrameUpdateCallbacks)),
          entityRegistry(std::move(other.entityRegistry)),
          fixedDT(other.fixedDT), remaining(other.remaining), entities(other.entities),
          componentCount(other.componentCount), componentCapacity(other.componentCapacity),
          entityCount(other.entityCount), entityCapacity(other.entityCapacity) {
        other.entities = nullptr;
        other.componentCount = nullptr;
        other.componentCapacity = nullptr;
        other.entityCount = 0u;
        other.entityCapacity = 0u;
    }

    ECS& operator=(ECS&& other) noexcept {
        if (this != &other) {
            free(entities);
            storages = std::move(other.storages);
            listeners = std::move(other.listeners);
            setupCallbacks = std::move(other.setupCallbacks);
            prePhysicsUpdateCallbacks = std::move(other.prePhysicsUpdateCallbacks);
            physicsUpdateCallbacks = std::move(other.physicsUpdateCallbacks);
            postPhysicsUpdateCallbacks = std::move(other.postPhysicsUpdateCallbacks);
            frameUpdateCallbacks = std::move(other.frameUpdateCallbacks);
            postFrameUpdateCallbacks = std::move(other.postFrameUpdateCallbacks);
            entityRegistry = std::move(other.entityRegistry);
            fixedDT = other.fixedDT;
            remaining = other.remaining;
            entities = other.entities;
            componentCount = other.componentCount;
            componentCapacity = other.componentCapacity;
            entityCount = other.entityCount;
            entityCapacity = other.entityCapacity;
            other.entities = nullptr;
            other.componentCount = nullptr;
            other.componentCapacity = nullptr;
            other.entityCount = 0u;
            other.entityCapacity = 0u;
        }
        return *this;
    }

    template <Component T, typename System, void (System::*OnAdded)(Entity, uint32_t), void (System::*OnRemoved)(Entity, uint32_t)>
    void registerComponentListener(System* system) {
        listeners[ComponentMetadata::typeOf<T>()].emplace(system, &Invoke<Entity, uint32_t>::thunk<System, OnAdded>, &Invoke<Entity, uint32_t>::thunk<System, OnRemoved>);
    }

    template <Component T, void (*OnAdded)(Entity, uint32_t), void (*OnRemoved)(Entity, uint32_t)>
    void registerComponentListener() {
        listeners[ComponentMetadata::typeOf<T>()].emplace(nullptr, &Invoke<Entity, uint32_t>::wrapThunk<OnAdded>, &Invoke<Entity, uint32_t>::wrapThunk<OnRemoved>);
    }

    template <typename System, void (System::*Setup)()>
    void registerSetupCallback(System* system) {
        setupCallbacks.emplace(system, &Invoke<>::thunk<System, Setup>);
    }

    template <void (*Setup)()>
    void registerSetupCallback() {
        setupCallbacks.emplace(nullptr, &Invoke<>::wrapThunk<Setup>);
    }

    template <typename System, void (System::*Update)(double), UpdateOrder Order>
    void registerUpdateCallback(System* system) {
        if constexpr (Order == UpdateOrder::PRE_PHYSICS) {
            prePhysicsUpdateCallbacks.emplace(system, &Invoke<double>::thunk<System, Update>);
        } else if constexpr (Order == UpdateOrder::PHYSICS) {
            physicsUpdateCallbacks.emplace(system, &Invoke<double>::thunk<System, Update>);
        } else if constexpr (Order == UpdateOrder::POST_PHYSICS) {
            postPhysicsUpdateCallbacks.emplace(system, &Invoke<double>::thunk<System, Update>);
        } else if constexpr (Order == UpdateOrder::FRAME) {
            frameUpdateCallbacks.emplace(system, &Invoke<double>::thunk<System, Update>);
        } else if constexpr (Order == UpdateOrder::POST_FRAME) {
            postFrameUpdateCallbacks.emplace(system, &Invoke<double>::thunk<System, Update>);
        }
    }

    template <void (*Update)(double), UpdateOrder Order>
    void registerUpdateCallback() {
        if constexpr (Order == UpdateOrder::PRE_PHYSICS) {
            prePhysicsUpdateCallbacks.emplace(nullptr, &Invoke<double>::wrapThunk<Update>);
        } else if constexpr (Order == UpdateOrder::PHYSICS) {
            physicsUpdateCallbacks.emplace(nullptr, &Invoke<double>::wrapThunk<Update>);
        } else if constexpr (Order == UpdateOrder::POST_PHYSICS) {
            postPhysicsUpdateCallbacks.emplace(nullptr, &Invoke<double>::wrapThunk<Update>);
        } else if constexpr (Order == UpdateOrder::FRAME) {
            frameUpdateCallbacks.emplace(nullptr, &Invoke<double>::wrapThunk<Update>);
        } else if constexpr (Order == UpdateOrder::POST_FRAME) {
            postFrameUpdateCallbacks.emplace(nullptr, &Invoke<double>::wrapThunk<Update>);
        }
    }

    Entity createEntity(const uint32_t initialCapacity = 8u) {
        ensureEntityCapacity(entityCount + 1u);
        entities[entityCount] = alloc<uint64_t>(initialCapacity);
        componentCapacity[entityCount] = initialCapacity;
        entityCount++;
        return Entity(entityRegistry.create(), this);
    }

    void deleteEntity(uint32_t entityID) {
        entityCount--;
        uint32_t loc;
        if (entityRegistry.remove(entityID, loc)) {
            uint32_t count = componentCount[loc];
            uint64_t* components = entities[loc];
            for (uint32_t i = 0u; i < count; i++) {
                uint64_t component = components[0u];
                reflectRemoveComponent(ECS::extractComponentType(component), entityID, ECS::extractComponentID(component));
            }
            free(components);
            entities[loc] = entities[entityCount];
            componentCount[loc] = componentCount[entityCount];
            componentCapacity[loc] = componentCapacity[entityCount];
        }
    }

    template <Component T>
    void addComponent(uint32_t entityID, const T& component) {
        static uint32_t type = ComponentMetadata::typeOf<T>();
        static uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
        Storage<T>* storage = getStorage<T>();
        uint32_t location = entityRegistry[entityID];
        uint32_t idx = componentCount[location]++;
        ensureComponentCapacity(location, componentCount[location]);
        uint32_t componentID = storage->add(component);
        entities[location][idx] = componentType | static_cast<uint64_t>(componentID);
        componentAdded(entityID, type, componentID);
    }

    template <Component T, T (*Create)()>
    void createAndAddComponent(uint32_t entityID) {
        addComponent(entityID, Create());
    }

    inline void reflectCreateAndAddComponent(uint32_t type, uint32_t entityID) {
        ComponentRegistry::reflection[type].createAndAddComponent(this, entityID);
    }

    template <Component T>
    bool setComponent(uint32_t entityID, const T& component) {
        static uint32_t type = ComponentMetadata::typeOf<T>();
        static uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
        Storage<T>* storage = getStorage<T>();
        uint32_t location = entityRegistry[entityID];
        uint64_t* entity = entities[location];
        uint32_t size = componentCount[location];
        for (uint32_t i = 0u; i < size; i++) {
            if ((entity[i] & TYPE_MASK) == componentType) {
                storage->set(static_cast<uint32_t>(entity[i] & ID_MASK), component);
                return true;
            }
        }
        return false;
    }

    template <Component T>
    void removeComponent(uint32_t entityID) {
        static uint32_t type = ComponentMetadata::typeOf<T>();
        static uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
        Storage<T>* storage = getStorage<T>();
        uint32_t location = entityRegistry[entityID];
        uint64_t* entity = entities[location];
        uint32_t size = componentCount[location];
        for (uint32_t i = 0u; i < size; i++) {
            if ((entity[i] & TYPE_MASK) == componentType) {
                uint32_t componentID = static_cast<uint32_t>(entity[i] & ID_MASK);
                componentRemoved(entityID, type, componentID);
                storage->remove(componentID);
                entity[i] = entity[--componentCount[location]];
                break;
            }
        }
    }

    inline void reflectRemoveComponent(uint32_t type, uint32_t entityID) {
        ComponentRegistry::reflection[type].removeComponent(this, entityID);
    }

    template <Component T>
    void removeComponent(uint32_t entityID, uint32_t componentID) {
        static uint32_t type = ComponentMetadata::typeOf<T>();
        Storage<T>* storage = getStorage<T>();
        uint32_t location = entityRegistry[entityID];
        uint64_t* entity = entities[location];
        uint32_t size = componentCount[location];
        for (uint32_t i = 0u; i < size; i++) {
            if (static_cast<uint32_t>(entity[i] & ID_MASK) == componentID) {
                componentRemoved(entityID, type, componentID);
                storage->remove(componentID);
                entity[i] = entity[--componentCount[location]];
                break;
            }
        }
    }

    inline void reflectRemoveComponent(uint32_t type, uint32_t entityID, uint32_t componentID) {
        ComponentRegistry::reflection[type].removeComponentByID(this, entityID, componentID);
    }

    template <Component T>
    bool getComponent(uint32_t entityID, T& out) {
        static uint32_t type = ComponentMetadata::typeOf<T>();
        static uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
        Storage<T>* storage = getStorage<T>();
        uint32_t location = entityRegistry[entityID];
        uint64_t* entity = entities[location];
        uint32_t size = componentCount[location];
        for (uint32_t i = 0u; i < size; i++) {
            if ((entity[i] & TYPE_MASK) == componentType) {
                out = storage->get(static_cast<uint32_t>(entity[i] & ID_MASK));
                return true;
            }
        }
        return false;
    }

    template <Component T>
    T* getComponentPtr(uint32_t entityID) {
        Storage<T>* storage = getStorage<T>();
        using ReturnT = decltype(storage->get(0));
        if constexpr (std::is_reference_v<ReturnT>) {
            static uint32_t type = ComponentMetadata::typeOf<T>();
            static uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
            uint32_t location = entityRegistry[entityID];
            uint64_t* entity = entities[location];
            uint32_t size = componentCount[location];
            for (uint32_t i = 0u; i < size; i++) {
                if ((entity[i] & TYPE_MASK) == componentType) {
                    return &storage->get(static_cast<uint32_t>(entity[i] & ID_MASK));
                }
            }
        }
        return nullptr;
    }

    inline void* reflectGetComponentPtr(uint32_t type, uint32_t entityID) {
        return ComponentRegistry::reflection[type].getComponentPtr(this, entityID);
    }

    template <Component T>
    uint32_t getComponentCount(uint32_t entityID) {
        static uint64_t componentType = static_cast<uint64_t>(ComponentMetadata::typeOf<T>()) << TYPE_SHIFT;
        uint32_t location = entityRegistry[entityID];
        uint64_t* entity = entities[location];
        uint32_t size = componentCount[location];
        uint32_t count = 0u;
        for (uint32_t i = 0u; i < size; i++) {
            if ((entity[i] & TYPE_MASK) == componentType) {
                count++;
            }
        }
        return count;
    }

    uint32_t reflectGetComponentCount(uint32_t type, uint32_t entityID) {
        uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
        uint32_t location = entityRegistry[entityID];
        uint64_t* entity = entities[location];
        uint32_t size = componentCount[location];
        uint32_t count = 0u;
        for (uint32_t i = 0u; i < size; i++) {
            if ((entity[i] & TYPE_MASK) == componentType) {
                count++;
            }
        }
        return count;
    }

    template <Component T>
    uint32_t getComponents(uint32_t entityID, T* const ptr) {
        static uint32_t type = ComponentMetadata::typeOf<T>();
        static uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
        Storage<T>* storage = getStorage<T>();
        uint32_t location = entityRegistry[entityID];
        uint64_t* entity = entities[location];
        uint32_t size = componentCount[location];
        uint32_t idx = 0u;
        for (uint32_t i = 0u; i < size; i++) {
            if ((entity[i] & TYPE_MASK) == componentType) {
                ptr[idx++] = storage->get(static_cast<uint32_t>(entity[i] & ID_MASK));
            }
        }
        return idx;
    }

    template <Component T>
    bool getComponentID(uint32_t entityID, uint32_t& componentID) {
        static uint32_t type = ComponentMetadata::typeOf<T>();
        static uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
        uint32_t location = entityRegistry[entityID];
        uint64_t* entity = entities[location];
        uint32_t size = componentCount[location];
        for (uint32_t i = 0u; i < size; i++) {
            if ((entity[i] & TYPE_MASK) == componentType) {
                componentID = static_cast<uint32_t>(entity[i] & ID_MASK);
                return true;
            }
        }
        return false;
    }

    bool reflectGetComponentID(uint32_t type, uint32_t entityID, uint32_t& componentID) {
        uint64_t componentType = static_cast<uint64_t>(type) << TYPE_SHIFT;
        uint32_t location = entityRegistry[entityID];
        uint64_t* entity = entities[location];
        uint32_t size = componentCount[location];
        for (uint32_t i = 0u; i < size; i++) {
            if ((entity[i] & TYPE_MASK) == componentType) {
                componentID = static_cast<uint32_t>(entity[i] & ID_MASK);
                return true;
            }
        }
        return false;
    }

    template <Component T>
    bool isComponentValid(uint32_t componentID) {
        return getStorage<T>()->valid(componentID);
    }

    inline bool reflectIsComponentValid(uint32_t type, uint32_t componentID) {
        return ComponentRegistry::reflection[type].isComponentValid(this, componentID);
    }

    template <Component T>
    T* getPtr(uint32_t componentID) {
        Storage<T>* storage = getStorage<T>();
        using ReturnT = decltype(storage->get(0));
        if constexpr (std::is_reference_v<ReturnT>) {
            return &storage->get(componentID);
        } else {
            return nullptr;
        }
    }

    inline void* reflectGetPtr(uint32_t type, uint32_t componentID) {
        return ComponentRegistry::reflection[type].getPtr(this, componentID);
    }

    template <Component T>
    T* getPtr(ComponentRef<T> ref) {
        return getPtr<T>(ref.ID);
    }

    template <Component T>
    T read(uint32_t componentID) {
        return getStorage<T>()->get(componentID);
    }

    template <Component T>
    T read(ComponentRef<T> ref) {
        return getStorage<T>()->get(ref.ID);
    }

    inline void reflectRead(uint32_t type, uint32_t componentID, void* dst) {
        ComponentRegistry::reflection[type].read(this, componentID, dst);
    }

    template <Component T>
    void write(uint32_t componentID, const T& component) {
        getStorage<T>()->set(componentID, component);
    }

    template <Component T>
    void write(ComponentRef<T> ref, const T& component) {
        getStorage<T>()->set(ref.ID, component);
    }

    inline void reflectWrite(uint32_t type, uint32_t componentID, void* src) {
        ComponentRegistry::reflection[type].write(this, componentID, src);
    }

    template <Component T>
    Storage<T>& view() {
        return *getStorage<T>();
    }

    void* reflectGetStorage(uint32_t type) {
        return storages[type];
    }

    void setup() {
        invokeAll<>(setupCallbacks);
    }

    void update(double dt) {
        invokeAll<double>(prePhysicsUpdateCallbacks, dt);
        remaining += dt;
        uint64_t start = Time::nanoTime();
        while (remaining >= fixedDT) {
            invokeAll<double>(physicsUpdateCallbacks, fixedDT);
            remaining -= fixedDT;
            if (Time::nanoTime() - start > 3000000ull) {
                remaining = 0.0;
                break;
            }
        }
        invokeAll<double>(postPhysicsUpdateCallbacks, dt);
        invokeAll<double>(frameUpdateCallbacks, dt);
        invokeAll<double>(postFrameUpdateCallbacks, dt);
    }

    void setPhysicsDT(double dt) {
        fixedDT = dt;
    }

    inline uint32_t getComponentCountRaw(uint32_t entityID) {
        return componentCount[entityRegistry[entityID]];
    }

    inline uint64_t* getComponentsRaw(uint32_t entityID) {
        return entities[entityRegistry[entityID]];
    }

    inline static uint32_t extractComponentType(uint64_t component) {
        return static_cast<uint32_t>((component & TYPE_MASK) >> TYPE_SHIFT);
    }

    inline static uint32_t extractComponentID(uint64_t component) {
        return static_cast<uint32_t>(component & ID_MASK);
    }
};

template <Component T>
ComponentView<T>::ComponentView(ECS* ecs, ComponentRef<T> ref) : ecs(ecs), ref(ref), temp(ecs->read(ref)) {
}

template <Component T>
ComponentView<T>::~ComponentView() {
    if (dirty) {
        ecs->write(ref, temp);
    }
}

template <Component T>
inline void Entity::addComponent(const T& component) {
    return ecs->addComponent<T>(entityID, component);
}

template <Component T>
inline bool Entity::setComponent(const T& component) {
    return ecs->setComponent<T>(entityID, component);
}

template <Component T>
inline void Entity::removeComponent() {
    return ecs->removeComponent<T>(entityID);
}

template <Component T>
inline bool Entity::getComponent(T& out) {
    return ecs->getComponent<T>(entityID, out);
}

template <Component T>
inline T* Entity::getComponentPtr() {
    return ecs->getComponentPtr<T>(entityID);
}

template <Component T>
inline uint32_t Entity::getComponentCount() {
    return ecs->getComponentCount<T>(entityID);
}

template <Component T>
inline uint32_t Entity::getComponents(T* const ptr) {
    return ecs->getComponents<T>(entityID, ptr);
}

template <Component T, T (*Create)()>
inline void ComponentRegistry::registerComponentType() {
    ComponentMetadata meta = ComponentMetadata::get<T>();
    while (meta.typeId >= metadata.size()) {
        metadata.add(ComponentMetadata::invalid());
        reflection.emplace();
    }
    metadata[meta.typeId] = meta;
    reflection[meta.typeId] = ComponentReflection{
        []() { return (void*) new Storage<T>{}; },
        [](void* storage) { delete reinterpret_cast<Storage<T>*>(storage); },
        reinterpret_cast<void (*)(void* ecs, uint32_t)>(&Invoke<uint32_t>::thunk<ECS, ECS::createAndAddComponent<T, Create>>),
        &Invoke<uint32_t>::thunk<ECS, ECS::removeComponent<T>>,
        &Invoke<uint32_t, uint32_t>::thunk<ECS, ECS::removeComponent<T>>,
        reinterpret_cast<void* (*) (void* ecs, uint32_t)>(&Invoke<uint32_t>::thunkReturn<ECS, T*, ECS::getComponentPtr<T>>),
        &Invoke<uint32_t>::thunkReturn<ECS, bool, ECS::isComponentValid<T>>,
        reinterpret_cast<void* (*) (void* ecs, uint32_t)>(&Invoke<uint32_t>::thunkReturn<ECS, T*, ECS::getPtr<T>>),
        &Invoke<uint32_t, void*>::thunk<ECS, ECS::read<T>>,
        &Invoke<uint32_t, void*>::thunk<ECS, ECS::write<T>>,
    };
}

#endif
