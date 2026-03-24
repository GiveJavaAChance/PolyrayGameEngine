#ifndef AUDIOFORMAT_H_INCLUDED
#define AUDIOFORMAT_H_INCLUDED

#pragma once
#include <cstdint>

struct AudioFormat {
    uint32_t sampleRate;
    uint16_t channels;
    uint16_t bitDepth;

    AudioFormat(uint32_t sampleRate_, uint16_t channels_, uint16_t bitDepth_) : sampleRate(sampleRate_), channels(channels_), bitDepth(bitDepth_) {}

    AudioFormat() : sampleRate(44100), channels(2), bitDepth(16) {}
};

#endif
