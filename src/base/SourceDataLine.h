#ifndef SOURCEDATALINE_H_INCLUDED
#define SOURCEDATALINE_H_INCLUDED

#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <cstring>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "AudioFormat.h"

#define MAX_BUFFER_SIZE 4096
#define QUEUE_SIZE 3

class SourceDataLine {
public:
    SourceDataLine(AudioFormat& format) : format(format), buffersInQueue(0) {
        for (int i = 0; i < QUEUE_SIZE; i++) {
            buffer[i] = new uint8_t[MAX_BUFFER_SIZE];
            headerQueue[i] = {};
        }
        WAVEFORMATEX waveFormat = {};
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.nChannels = format.channels;
        waveFormat.nSamplesPerSec = format.sampleRate;
        waveFormat.wBitsPerSample = format.bitDepth;
        waveFormat.nBlockAlign = (format.channels * format.bitDepth) / 8;
        waveFormat.nAvgBytesPerSec = format.sampleRate * waveFormat.nBlockAlign;
        waveFormat.cbSize = 0;

        if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormat, (DWORD_PTR)&waveOutCallback, (DWORD_PTR)this, CALLBACK_FUNCTION)) {
            std::cerr << "Error opening audio output device!" << std::endl;
            exit(1);
        }
    }

    ~SourceDataLine() {
        for (int i = 0; i < QUEUE_SIZE; i++) {
            delete[] buffer[i];
        }
        for (int i = 0; i < buffersInQueue; i++) {
            if (headerQueue[i].dwFlags & WHDR_PREPARED) {
                waveOutUnprepareHeader(hWaveOut, &headerQueue[i], sizeof(WAVEHDR));
            }
        }
        waveOutClose(hWaveOut);
    }

    void write(const uint8_t* audioData, size_t dataSize) {
        if (dataSize > MAX_BUFFER_SIZE) {
            std::cerr << "Error: Data size exceeds buffer limit!" << std::endl;
            return;
        }
        std::unique_lock<std::mutex> lock(mtx);

        condVar.wait(lock, [&]() {
            return buffersInQueue < QUEUE_SIZE;
        });

        std::memcpy(buffer[buffersInQueue], audioData, dataSize);

        WAVEHDR& waveHeader = headerQueue[buffersInQueue];
        waveHeader.lpData = (LPSTR)buffer[buffersInQueue];
        waveHeader.dwBufferLength = (DWORD)dataSize;
        waveHeader.dwFlags = 0;

        if (waveOutPrepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
            std::cerr << "Error preparing wave header!" << std::endl;
            return;
        }

        buffersInQueue++;

        if (waveOutWrite(hWaveOut, &waveHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
            std::cerr << "Error writing to audio device!" << std::endl;
            return;
        }
    }

private:
    HWAVEOUT hWaveOut;
    AudioFormat& format;
    int buffersInQueue;
    WAVEHDR headerQueue[QUEUE_SIZE];
    uint8_t* buffer[QUEUE_SIZE];

    std::mutex mtx;
    std::condition_variable condVar;

    static void CALLBACK waveOutCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
        if (uMsg == WOM_DONE) {

            SourceDataLine* self = reinterpret_cast<SourceDataLine*>(dwInstance);
            WAVEHDR* doneHeader = reinterpret_cast<WAVEHDR*>(dwParam1);

            std::unique_lock<std::mutex> lock(self->mtx);
            waveOutUnprepareHeader(hwo, doneHeader, sizeof(WAVEHDR));
            for (int i = 0; i < QUEUE_SIZE - 1; i++) {
                self->headerQueue[i] = self->headerQueue[i + 1];
                self->buffer[i] = self->buffer[i + 1];
            }
            self->buffersInQueue--;

            lock.unlock();
            self->condVar.notify_one();
        }
    }
};

#endif
