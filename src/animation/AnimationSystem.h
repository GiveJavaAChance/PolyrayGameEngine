#ifndef ANIMATIONSYSTEM_H_INCLUDED
#define ANIMATIONSYSTEM_H_INCLUDED

#include "ecs/ComponentMetadata.h"
#include "ecs/ECS.h"
#include "utils/perf.h"
#include <malloc.h>
#include <stdexcept>
#pragma once

#include <cstdint>
#include <unordered_map>

#include <Profiler.h>
#include <Transform3D.h>
#include <World.h>
#include <animation/Animation.h>
#include <animation/AnimationBinding.h>
#include <animation/AnimationChannel.h>
#include <ecs/Export.h>
#include <scene/3d/Scene3D.h>
#include <serial/Serial.h>
#include <structure/BitSet.h>
#include <structure/DynamicArray.h>
#include <structure/UnorderedRegistry.h>

#include <utils/prvl_assert.h>

struct AnimationSystem;

struct AnimationInstance {
    uint32_t groupID;
    uint32_t animationID = UINT32_MAX;

    float speed = 1.0f;
    bool repeat = false;

    float time = 0.0f;
    bool playing = false;

    bool dirty = true;

    uint32_t cacheID = UINT32_MAX;
    uint32_t entityID = UINT32_MAX;

    AnimationSystem* animationSystem = nullptr;

    AnimationInstance() {
    }

    AnimationInstance(uint32_t groupID) : groupID(groupID) {
    }

    inline void play() {
        playing = true;
    }

    inline void play(const std::string& animationName, bool repeat = false, float speed = 1.0f);

    inline void pause() {
        playing = false;
    }

    inline void stop() {
        playing = false;
        time = 0.0f;
        dirty = true;
    }

    inline const DynamicArray<std::string>& getAnimations();
};

template <>
struct ExportInfo<AnimationInstance> {
    constexpr static Export __export__[] = {
        {offsetof(AnimationInstance, speed), EXPORT_FLOAT, "Speed"},
        {offsetof(AnimationInstance, repeat), EXPORT_BOOL, "Repeats"},
        {offsetof(AnimationInstance, time), EXPORT_FLOAT, "Current Time"},
        {offsetof(AnimationInstance, playing), EXPORT_BOOL, "Playing"},
    };
};

template <>
struct Serial<AnimationInstance> {
    constexpr static uint32_t SIZE = offsetof(AnimationInstance, cacheID);

    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        AnimationInstance* instance = world->ecs.getPtr<AnimationInstance>(componentID);
        output.write(instance, SIZE);
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        AnimationInstance instance{};
        input.read(&instance, SIZE);
        e.addComponent(instance);
    }
};

struct AnimationSystem {
private:
    struct AnimationChannelGroup {
        uint32_t componentType;
        uint32_t componentID;
        DynamicArray<uint32_t> channels;
        void* buffer = nullptr;

        AnimationChannelGroup() {
        }

        ~AnimationChannelGroup() {
            if (buffer) {
                _mm_free(buffer);
            }
        }

        AnimationChannelGroup(const AnimationChannelGroup& other) = delete;
        AnimationChannelGroup& operator=(const AnimationChannelGroup& other) = delete;

        AnimationChannelGroup(AnimationChannelGroup&& other) : componentType(other.componentType), componentID(other.componentID), channels(std::move(other.channels)), buffer(other.buffer) {
            other.buffer = nullptr;
        }

        AnimationChannelGroup& operator=(AnimationChannelGroup&& other) noexcept {
            if (this != &other) {
                if (buffer) {
                    _mm_free(buffer);
                }
                componentType = other.componentType;
                componentID = other.componentID;
                buffer = other.buffer;
                channels = std::move(other.channels);
                other.buffer = nullptr;
            }
            return *this;
        }
    };

    struct AnimationCache {
        uint32_t animationID;
        DynamicArray<AnimationChannelGroup> groups;
        DynamicArray<uint32_t> keyframes;
        bool cold = true;
    };

    struct AnimationGroup {
        std::string name;
        DynamicArray<std::string> animationNames;
        std::unordered_map<std::string, uint32_t> animationNameMap;
    };

    World* world;

    UnorderedRegistry<AnimationGroup> animationGroups;
    std::unordered_map<std::string, uint32_t> groupNameMap;

    std::unordered_map<uint32_t, AnimationCache> animationCache;

    inline void onAnimationAdded(Entity e, uint32_t id) {
        AnimationInstance* instance = world->ecs.getPtr<AnimationInstance>(id);
        instance->entityID = e.entityID;
        instance->cacheID = id;
        instance->animationSystem = this;
    }

    inline void onAnimationRemoved(Entity e, uint32_t id) {
        animationCache.erase(id);
    }

    void applyAnimation(const Animation& animation, const AnimationInstance& instance, Scene3D* scene, uint32_t node) {
        uint32_t channelCount = animation.channels.size();
        AnimationCache& cache = animationCache[instance.cacheID];
        if (cache.animationID != instance.animationID) {
            cache.animationID = instance.animationID;
            cache.groups.clear();
            cache.cold = true;
        }
        if (cache.cold) {
            cache.cold = false;
            cache.keyframes.ensureCapacity(channelCount);
            DynamicArray<uint32_t> componentIDs(channelCount);
            for (uint32_t i = 0u; i < channelCount; i++) {
                AnimationChannel& channel = animation.channels[i];
                AnimationBinding& binding = channel.binding;

                uint32_t targetNode = scene->getNode(node, binding.targetPath.c_str());
                Entity targetEntity = scene->getEntity(targetNode);
                world->ecs.reflectGetComponentID(binding.componentType, targetEntity.entityID, componentIDs[i]);

                cache.keyframes[i] = UINT32_MAX;
            }

            BitSet mask;
            for (uint32_t i = 0u; i < channelCount; i++) {
                if (mask[i]) {
                    continue;
                }
                mask.set(i);
                AnimationChannelGroup& group = cache.groups.emplace();
                group.componentID = componentIDs[i];
                group.channels.add(i);
                group.componentType = animation.channels[i].binding.componentType;
                const ComponentMetadata& meta = ComponentRegistry::metadata[group.componentType];
                group.buffer = _mm_malloc(meta.size, meta.alignment);
                for (uint32_t j = i + 1u; j < channelCount; j++) {
                    if (mask[j]) {
                        continue;
                    }
                    if (componentIDs[j] == group.componentID) {
                        mask.set(j);
                        group.channels.add(j);
                    }
                }
            }
        }
        for (uint32_t i = 0u; i < cache.groups.size(); i++) {
            world->threadPool.submit([&, i]() {
                AnimationChannelGroup& group = cache.groups[i];
                void* buffer = world->ecs.reflectGetPtr(group.componentType, group.componentID);
                bool write = false;
                if (!buffer) {
                    write = true;
                    buffer = group.buffer;
                    world->ecs.reflectRead(group.componentType, group.componentID, buffer);
                }
                for (uint32_t j = 0u; j < group.channels.size(); j++) {
                    uint32_t channeldIdx = group.channels[j];

                    AnimationChannel& channel = animation.channels[channeldIdx];
                    AnimationBinding& binding = channel.binding;

                    cache.keyframes[channeldIdx] = channel.sample(instance.time, reinterpret_cast<uint8_t*>(buffer) + binding.memberOffset, cache.keyframes[channeldIdx]);
                    if (binding.dirtyMask) {
                        uint32_t flagData = 0u;
                        void* dirty = reinterpret_cast<uint8_t*>(buffer) + binding.dirtyOffset;
                        std::memcpy(&flagData, dirty, binding.dirtySize);
                        flagData |= binding.dirtyMask;
                        std::memcpy(dirty, &flagData, binding.dirtySize);
                    }
                }
                if (write) {
                    world->ecs.reflectWrite(group.componentType, group.componentID, buffer);
                }
            });
        }
    }

public:
    UnorderedRegistry<Animation> animations;

    AnimationSystem(World* world) : world(world) {
        world->ecs.registerComponentListener<AnimationInstance, AnimationSystem, &AnimationSystem::onAnimationAdded, &AnimationSystem::onAnimationRemoved>(this);
        world->ecs.registerUpdateCallback<AnimationSystem, &AnimationSystem::update, UpdateOrder::POST_FRAME>(this);
    }

    inline uint32_t createGroup(const std::string& name) {
        uint32_t id = animationGroups.emplace(name);
        groupNameMap[name] = id;
        return id;
    }

    inline uint32_t getGroup(const std::string& name) {
        auto it = groupNameMap.find(name);
        PRVL_ASSERT(it != groupNameMap.end(), "No group with name \"" + name + "\" found.")
        return it->second;
    }

    inline uint32_t createAnimation(uint32_t groupID, const std::string& name) {
        uint32_t id = animations.emplace(name, groupID);
        AnimationGroup& group = animationGroups[groupID];
        group.animationNameMap[name] = id;
        group.animationNames.add(name);
        return id;
    }

    inline Animation& getAnimation(uint32_t animationID) {
        PRVL_ASSERT(animations.reg.valid(animationID), "Invalid animation ID")
        return animations[animationID];
    }

    inline uint32_t getAnimationByName(uint32_t groupID, const std::string& name) {
        PRVL_ASSERT(animationGroups.reg.valid(groupID), "Invalid group ID")
        AnimationGroup& group = animationGroups[groupID];
        auto it = group.animationNameMap.find(name);
        PRVL_ASSERT(it != group.animationNameMap.end(), "No animation with name \"" + name + "\" in group " + group.name + " found.")
        return it->second;
    }

    inline const DynamicArray<std::string>& getAnimations(uint32_t groupID) {
        PRVL_ASSERT(animationGroups.reg.valid(groupID), "Invalid group ID")
        return animationGroups[groupID].animationNames;
    }

    void update(double dt) {
        PROFILE_SCOPE(AnimationSystem_Update)
        DynamicArray<AnimationInstance>& animationInstances = world->ecs.view<AnimationInstance>().data;
        for (uint32_t i = 0u; i < animationInstances.size(); i++) {
            AnimationInstance& instance = animationInstances[i];
            if (!animations.reg.valid(instance.animationID)) {
                continue;
            }
            Animation& animation = animations[instance.animationID];
            bool update = instance.dirty;
            instance.dirty = false;
            if (instance.playing) {
                instance.time += static_cast<float>(dt) * instance.speed;
                if (instance.time >= animation.duration || instance.time < 0.0f) {
                    if (instance.repeat) {
                        instance.time = mod(instance.time, animation.duration);
                        if (instance.time < 0.0f) {
                            instance.time += animation.duration;
                        }
                    } else {
                        instance.time = clamp(instance.time, 0.0f, animation.duration);
                        instance.playing = false;
                    }
                }
                update = true;
            }
            if (update) {
                Scene3D* scene = world->getSystem<Scene3D>();
                uint32_t node = scene->getNode(instance.entityID);
                applyAnimation(animation, instance, scene, node);
            }
        }
        world->threadPool.barrier();
    }
};

void AnimationInstance::play(const std::string& animationName, bool repeat, float speed) {
    uint32_t animationID = animationSystem->getAnimationByName(groupID, animationName);
    this->animationID = animationID;
    this->speed = speed;
    this->repeat = repeat;
    time = 0.0f;
    playing = true;
}

const DynamicArray<std::string>& AnimationInstance::getAnimations() {
    return animationSystem->getAnimations(groupID);
}

#endif