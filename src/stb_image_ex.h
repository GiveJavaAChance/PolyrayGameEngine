#ifndef STB_IMAGE_EX_H_INCLUDED
#define STB_IMAGE_EX_H_INCLUDED

#pragma once

#include "stb_image.h"

/*extern "C" {
    unsigned char* stbi_load_wrapper(const char* filename, int* x, int* y, int* channels_in_file, int desired_channels);
    void stbi_image_free_wrapper(void* retval_from_stbi_load);
}*/

inline unsigned char* stbi_load_wrapper(const char* filename, int* x, int* y, int* channels_in_file, int desired_channels) {
    return stbi_load(filename, x, y, channels_in_file, desired_channels);
}

inline void stbi_image_free_wrapper(void* data) {
    stbi_image_free(data);
}

#endif
