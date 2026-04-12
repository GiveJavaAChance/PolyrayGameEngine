#include <cstring>
#include <cstdint>
#include <immintrin.h>

#define STBI_MALLOC(sz)   _mm_malloc(sz, 32)
#define STBI_FREE(p)      _mm_free(p)

void* _mm_remalloc(void* ptr, size_t oldSize, size_t newSize) {
    void* newPtr = _mm_malloc(newSize, 32u);
    if (ptr) {
        size_t copySize = oldSize < newSize ? oldSize : newSize;
        std::memcpy(newPtr, ptr, copySize);
        _mm_free(ptr);
    }
    return newPtr;
}
#define STBI_REALLOC_SIZED(p, oldsz, newsz) _mm_remalloc(p, oldsz, newsz)

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
