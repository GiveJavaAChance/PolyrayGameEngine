#include "polyray_audio_NativeDCDBREffect.h"
#include <cstdint>
#include <cstring>
#include <immintrin.h>

float* collectorRight = nullptr;
float* collectorLeft = nullptr;
int collectorSize = 0;

float* volumesR = nullptr;
float* volumesL = nullptr;
uint32_t* delaysI = nullptr;
int sampleCount = 0;

void DBR(float* buffer, int& bufferSize, float* outputRight, float* outputLeft) {
    for (int s = 0; s < sampleCount; s++) {
        int delay = delaysI[s];
        float vR = volumesR[s];
        float vL = volumesL[s];

        float* dstR = collectorRight + delay;
        float* dstL = collectorLeft + delay;

        float* src = buffer;

        int unaligned = (8 - (delay & 7)) & 7;
        int processed = 0;

        __m256 mulR = _mm256_set1_ps(vR);
        __m256 mulL = _mm256_set1_ps(vL);

        for (int i = 0; i < unaligned && i < bufferSize; i++) {
            dstR[i] += src[i] * vR;
            dstL[i] += src[i] * vL;
            processed++;
        }

        dstR += processed;
        dstL += processed;
        src  += processed;
        int simdEnd = (bufferSize - processed) & ~7;

        #pragma GCC unroll 4
        for (int i = 0; i < simdEnd; i += 8) {
            __m256 in = _mm256_loadu_ps(src);
            _mm256_store_ps(dstR, _mm256_fmadd_ps(in, mulR, _mm256_load_ps(dstR)));
            _mm256_store_ps(dstL, _mm256_fmadd_ps(in, mulL, _mm256_load_ps(dstL)));
            dstR += 8;
            dstL += 8;
            src  += 8;
        }

        int tailStart = processed + simdEnd;
        for (int i = tailStart; i < bufferSize; i++) {
            collectorRight[delay + i] += buffer[i] * vR;
            collectorLeft[delay + i] += buffer[i] * vL;
        }
    }

    int len = bufferSize * sizeof(float);

    std::memcpy(outputRight, collectorRight, len);
    std::memcpy(outputLeft, collectorLeft, len);

    std::memmove(collectorRight, collectorRight + bufferSize, (collectorSize - bufferSize) * sizeof(float));
    std::memmove(collectorLeft, collectorLeft + bufferSize, (collectorSize - bufferSize) * sizeof(float));

    std::memset(collectorRight + (collectorSize - bufferSize), 0, len);
    std::memset(collectorLeft + (collectorSize - bufferSize), 0, len);
}

float* alloc(int length) {
    return static_cast<float*>(_mm_malloc(length * sizeof(float), 32));
}

JNIEXPORT void JNICALL Java_polyray_audio_NativeDCDBREffect_setCollectorSize
(JNIEnv*, jobject, jint length) {
    if (collectorRight) _mm_free(collectorRight);
    if (collectorLeft) _mm_free(collectorLeft);

    collectorSize = length;
    collectorRight = static_cast<float*>(_mm_malloc(collectorSize * sizeof(float), 32));
    collectorLeft = static_cast<float*>(_mm_malloc(collectorSize * sizeof(float), 32));

    std::memset(collectorRight, 0, collectorSize * sizeof(float));
    std::memset(collectorLeft, 0, collectorSize * sizeof(float));
}

JNIEXPORT void JNICALL Java_polyray_audio_NativeDCDBREffect_applyDBR
(JNIEnv* env, jobject, jfloatArray buffer, jfloatArray rightChannel, jfloatArray leftChannel, jint bufferSize) {
    jfloat* buffRaw = env->GetFloatArrayElements(buffer, nullptr);
    jfloat* outRightRaw = env->GetFloatArrayElements(rightChannel, nullptr);
    jfloat* outLeftRaw = env->GetFloatArrayElements(leftChannel, nullptr);

    int len = bufferSize * sizeof(float);

    float* buff = static_cast<float*>(_mm_malloc(len, 32));
    float* outRight = static_cast<float*>(_mm_malloc(len, 32));
    float* outLeft = static_cast<float*>(_mm_malloc(len, 32));

    std::memcpy(buff, buffRaw, len);

    int bufferSizeInt = static_cast<int>(bufferSize);
    DBR(buff, bufferSizeInt, outRight, outLeft);

    std::memcpy(outRightRaw, outRight, len);
    std::memcpy(outLeftRaw, outLeft, len);

    env->ReleaseFloatArrayElements(rightChannel, outRightRaw, 0);
    env->ReleaseFloatArrayElements(leftChannel, outLeftRaw, 0);
    env->ReleaseFloatArrayElements(buffer, buffRaw, 0);

    _mm_free(buff);
    _mm_free(outRight);
    _mm_free(outLeft);
}

JNIEXPORT void JNICALL Java_polyray_audio_NativeDCDBREffect_setSamples
(JNIEnv* env, jobject, jfloatArray volumesRight, jfloatArray volumesLeft, jintArray delays, jint numSamples) {
    if (volumesR) _mm_free(volumesR);
    if (volumesL) _mm_free(volumesL);
    if (delaysI) _mm_free(delaysI);

    volumesR = static_cast<float*>(_mm_malloc(numSamples * sizeof(float), 32));
    volumesL = static_cast<float*>(_mm_malloc(numSamples * sizeof(float), 32));
    delaysI  = static_cast<uint32_t*>(_mm_malloc(numSamples * sizeof(uint32_t), 32));

    int len = numSamples * sizeof(float);

    jfloat* vL = env->GetFloatArrayElements(volumesLeft, nullptr);
    jfloat* vR = env->GetFloatArrayElements(volumesRight, nullptr);
    jint* dI = env->GetIntArrayElements(delays, nullptr);

    if (!vL || !vR || !dI) return;

    std::memcpy(volumesR, vR, len);
    std::memcpy(volumesL, vL, len);
    std::memcpy(delaysI, dI, numSamples * sizeof(uint32_t));

    sampleCount = numSamples;

    env->ReleaseFloatArrayElements(volumesLeft, vL, 0);
    env->ReleaseFloatArrayElements(volumesRight, vR, 0);
    env->ReleaseIntArrayElements(delays, dI, 0);
}
