#ifndef ANIMATIONCHANNEL_H_INCLUDED
#define ANIMATIONCHANNEL_H_INCLUDED

#pragma once

#include <animation/AnimationBinding.h>
#include <animation/Interpolation.h>
#include <structure/DynamicArray.h>

struct AnimationChannel {
private:
    template <typename T>
    inline static void extractSame(const T* in, T* out) {
        *out = *in;
    }

public:
    AnimationBinding binding;

    Interpolation interpolation;
    void (*interpolator)(const void*, const void*, void*, float, float);
    void (*extractor)(const void*, void*);
    uint32_t keyframeSize;

    DynamicArray<float> keyframeTimes;
    DynamicArray<uint8_t> keyframes;

    template <typename T>
    void setInterpolator(void (*interpolate)(const T* a, const T* b, T* c, float t, float dt)) {
        keyframeSize = sizeof(T);
        interpolator = reinterpret_cast<void (*)(const void*, const void*, void*, float, float)>(interpolate);
        extractor = reinterpret_cast<void (*)(const void*, void*)>(&AnimationChannel::extractSame<T>);
    }

    template <typename T, typename U>
    void setInterpolator(void (*interpolate)(const T* a, const T* b, U* c, float t, float dt), void (*extract)(const T* in, U* out)) {
        keyframeSize = sizeof(T);
        interpolator = reinterpret_cast<void (*)(const void*, const void*, void*, float, float)>(interpolate);
        extractor = reinterpret_cast<void (*)(const void*, void*)>(extract);
    }

    void addKeyframe(float time, const void* value) {
        keyframeTimes.add(time);
        keyframes.addAll(reinterpret_cast<const uint8_t*>(value), keyframeSize);
    }

    inline uint32_t sample(float time, void* dst, uint32_t current = UINT32_MAX) const {
        uint32_t count = keyframeTimes.size();
        if (count == 0u) {
            return UINT32_MAX;
        }
        if (time < keyframeTimes[0u]) {
            extractor(keyframes.data(), dst);
            return UINT32_MAX;
        }
        if (time >= keyframeTimes[count - 1u]) {
            uint32_t idx = count - 1u;
            extractor(keyframes.data() + idx * keyframeSize, dst);
            return idx;
        }
        if (current == UINT32_MAX || time < keyframeTimes[current]) {
            uint32_t low = 0u;
            uint32_t high = current == UINT32_MAX ? count : current;

            while (low < high) {
                uint32_t middle = low + (high - low) / 2u;
                if (keyframeTimes[middle] <= time) {
                    low = middle + 1u;
                } else {
                    high = middle;
                }
            }
            current = low - 1u;
        } else {
            while (current + 1u < count && time >= keyframeTimes[current + 1u]) {
                current++;
            }
        }
        if (current == UINT32_MAX) {
            extractor(keyframes.data(), dst);
            return current;
        }
        uint32_t currentOffset = current * keyframeSize;
        if (interpolation == Interpolation::STEP) {
            extractor(keyframes + currentOffset, dst);
            return current;
        }
        uint32_t next = current + 1u;
        uint32_t nextOffset = next * keyframeSize;
        float dt = keyframeTimes[next] - keyframeTimes[current];
        float t = (time - keyframeTimes[current]) / dt;
        interpolator(keyframes + currentOffset, keyframes + nextOffset, dst, t, dt);
        return current;
    }
};

#endif