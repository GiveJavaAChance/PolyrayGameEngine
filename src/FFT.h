#ifndef FFT_H_INCLUDED
#define FFT_H_INCLUDED

#pragma once

#include <cstdint>
#include <math.h>

#include "Allocator.h"

namespace FFT {
    namespace {
        void transpose(float* real, float* imag, uint32_t size) {
            for (uint32_t i = 0u; i < size; i++) {
                uint32_t ii = i * size;
                for (uint32_t j = i + 1u; j < size; j++) {
                    uint32_t a = ii + j;
                    uint32_t b = i + j * size;
                    float tmpRe = real[a];
                    float tmpIm = imag[a];
                    real[a] = real[b];
                    imag[a] = imag[b];
                    real[b] = tmpRe;
                    imag[b] = tmpIm;
                }
            }
        }
    }

    void fft(float* real, float* imag, uint32_t n) {
        if (n == 1u) {
            return;
        }
        uint32_t size = n >> 1u;
        float* evenRe = alloc<float>(n << 1u);
        float* evenIm = evenRe + size;
        float* oddRe = evenIm + size;
        float* oddIm = oddRe + size;
        for (uint32_t i = 0u; i < size; i++) {
            uint32_t idx = i << 1u;
            evenRe[i] = real[idx];
            evenIm[i] = imag[idx];
            oddRe[i] = real[idx + 1];
            oddIm[i] = imag[idx + 1];
        }

        fft(evenRe, evenIm, size);
        fft(oddRe, oddIm, size);

        float th = -6.28318530718f;
        for (uint32_t k = 0; k < size; k++) {
            float kth = static_cast<float>(k) * th / static_cast<float>(n);
            float s = std::sin(kth);
            float c = std::cos(kth);

            float eRe = evenRe[k];
            float eIm = evenIm[k];
            float oRe = oddRe[k];
            float oIm = oddIm[k];
            float wkRe = c * oRe - s * oIm;
            float wkIm = c * oIm + s * oRe;

            real[k] = eRe + wkRe;
            imag[k] = eIm + wkIm;
            real[k + size] = eRe - wkRe;
            imag[k + size] = eIm - wkIm;
        }
        free(evenRe);
    }

    void ifft(float* real, float* imag, uint32_t n) {
        for (uint32_t i = 0u; i < n; i++) {
            imag[i] = -imag[i];
        }
        fft(real, imag, n);
        float k = 1.0f / static_cast<float>(n);
        for (uint32_t i = 0u; i < n; i++) {
            real[i] = real[i] * k;
            imag[i] = -imag[i] * k;
        }
    }

    void fft2D(float* real, float* imag, uint32_t size) {
        for (uint32_t i = 0u; i < size; i++) {
            fft(real + i * size, imag + i * size, size);
        }
        transpose(real, imag, size);
        for (uint32_t i = 0u; i < size; i++) {
            fft(real + i * size, imag + i * size, size);
        }
        transpose(real, imag, size);
    }

    void ifft2D(float* real, float* imag, uint32_t size) {
        for (uint32_t i = 0u; i < size; i++) {
            ifft(real + i * size, imag + i * size, size);
        }
        transpose(real, imag, size);
        for (uint32_t i = 0u; i < size; i++) {
            ifft(real + i * size, imag + i * size, size);
        }
        transpose(real, imag, size);
    }
}

#endif
