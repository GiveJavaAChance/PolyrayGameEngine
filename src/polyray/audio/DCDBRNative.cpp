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
    for(int i = 0; i < sampleCount; i++) {
        int delay = delaysI[i];

        float* dstR = collectorRight + delay;
        float* dstL = collectorLeft + delay;

        __m256 mulR = _mm256_set1_ps(volumesR[i]);
        __m256 mulL = _mm256_set1_ps(volumesL[i]);
        #pragma GCC unroll 8
        for (int i = 0; i < bufferSize; i += 8) {
            float* src = buffer + i;
            __m256 inVec = _mm256_loadu_ps(src);
            _mm256_storeu_ps(dstR, _mm256_fmadd_ps(inVec, mulR, _mm256_loadu_ps(dstR)));
            _mm256_storeu_ps(dstL, _mm256_fmadd_ps(inVec, mulL, _mm256_loadu_ps(dstL)));
            dstR += 8;
            dstL += 8;
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

JNIEXPORT void JNICALL Java_polyray_audio_NativeDCDBREffect_setCollectorSize
(JNIEnv*, jobject, jint length) {
    free(collectorRight);
    free(collectorLeft);

    collectorSize = length;
    collectorRight = static_cast<float*>(malloc(collectorSize * sizeof(float)));
    collectorLeft = static_cast<float*>(malloc(collectorSize * sizeof(float)));

    std::memset(collectorRight, 0, collectorSize * sizeof(float));
    std::memset(collectorLeft, 0, collectorSize * sizeof(float));
}

JNIEXPORT void JNICALL Java_polyray_audio_NativeDCDBREffect_applyDBR
(JNIEnv* env, jobject, jfloatArray buffer, jfloatArray rightChannel, jfloatArray leftChannel, jint bufferSize) {
    jfloat* buff = env->GetFloatArrayElements(buffer, nullptr);
    jfloat* outRight = env->GetFloatArrayElements(rightChannel, nullptr);
    jfloat* outLeft = env->GetFloatArrayElements(leftChannel, nullptr);

    int bufferSizeInt = static_cast<int>(bufferSize);
    DBR(buff, bufferSizeInt, outRight, outLeft);

    env->ReleaseFloatArrayElements(rightChannel, outRight, 0);
    env->ReleaseFloatArrayElements(leftChannel, outLeft, 0);
    env->ReleaseFloatArrayElements(buffer, buff, 0);
}

JNIEXPORT void JNICALL Java_polyray_audio_NativeDCDBREffect_setSamples
(JNIEnv* env, jobject, jfloatArray volumesRight, jfloatArray volumesLeft, jintArray delays, jint numSamples) {
    free(volumesR);
    free(volumesL);
    free(delaysI);

    volumesR = static_cast<float*>(malloc(numSamples * sizeof(float)));
    volumesL = static_cast<float*>(malloc(numSamples * sizeof(float)));
    delaysI = static_cast<uint32_t*>(malloc(numSamples * sizeof(uint32_t)));

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
