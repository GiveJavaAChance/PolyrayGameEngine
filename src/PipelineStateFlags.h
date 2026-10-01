#ifndef PIPELINESTATEFLAGS_H_INCLUDED
#define PIPELINESTATEFLAGS_H_INCLUDED

#pragma once

#include <cstdint>

enum PipelineStateFlags : uint64_t {
    BACKFACE_CULLING = 1ull,
    ALPHA_BLEND = 0b10ull,
    ALPHA_TO_COVER = 0b100ull
};

#endif
