#ifndef THREADPOOL_H_INCLUDED
#define THREADPOOL_H_INCLUDED

#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>

template <uint32_t ThreadCount, uint32_t BufferSize = 1024u>
struct ThreadPool {
    static_assert((BufferSize & (BufferSize - 1u)) == 0u, "Buffer size must be a power of 2");

private:
    constexpr static uint32_t MASK = BufferSize - 1u;

    enum State : uint8_t {
        EMPTY = 0u,
        READY = 1u,
        BUSY = 2u
    };

    struct alignas(64u) Task {
        std::atomic<uint8_t> state{State::EMPTY};
        std::function<void()> func;
    };

    Task tasks[BufferSize];
    std::thread threads[ThreadCount];

    alignas(64u) std::atomic<uint32_t> writeIndex{0u};
    alignas(64u) std::atomic<uint32_t> readIndex{0u};
    alignas(64u) std::atomic<uint32_t> pendingTasks{0u};

    std::atomic<bool> running{false};
    std::atomic<bool> done{false};

    std::mutex stateMutex;
    std::condition_variable stateCondition;

    void thread() {
        while (!done.load(std::memory_order_relaxed)) {
            if (!running.load(std::memory_order_relaxed)) {
                std::unique_lock<std::mutex> lock(stateMutex);
                stateCondition.wait(lock, [&] {
                    return running.load(std::memory_order_relaxed) || done.load(std::memory_order_relaxed);
                });
                if (done.load(std::memory_order_relaxed)) {
                    break;
                }
            }
            uint32_t idx = readIndex.fetch_add(1u, std::memory_order_relaxed) & MASK;
            Task& task = tasks[idx];
            uint8_t expected = State::READY;
            if (task.state.compare_exchange_strong(expected, State::BUSY, std::memory_order_acquire, std::memory_order_relaxed)) {
                task.func();
                task.func = nullptr;
                task.state.store(State::EMPTY, std::memory_order_release);
                pendingTasks.fetch_sub(1u, std::memory_order_release);
            } else {
                std::this_thread::yield();
            }
        }
    }

public:
    ThreadPool() {
        for (uint32_t i = 0u; i < BufferSize; ++i) {
            tasks[i].state.store(State::EMPTY, std::memory_order_relaxed);
        }
        for (uint32_t i = 0u; i < ThreadCount; ++i) {
            threads[i] = std::thread(&ThreadPool::thread, this);
        }
    }

    ~ThreadPool() {
        done.store(true, std::memory_order_release);
        running.store(false, std::memory_order_release);
        stateCondition.notify_all();
        for (uint32_t i = 0u; i < ThreadCount; ++i) {
            if (threads[i].joinable()) {
                threads[i].join();
            }
        }
    }

    void start() {
        running.store(true, std::memory_order_release);
        stateCondition.notify_all();
    }

    void stop() {
        running.store(false, std::memory_order_release);
    }

    void submit(std::function<void()> func) {
        pendingTasks.fetch_add(1u, std::memory_order_relaxed);
        while (true) {
            uint32_t idx = writeIndex.fetch_add(1u, std::memory_order_relaxed) & MASK;
            Task& task = tasks[idx];
            if (task.state.load(std::memory_order_acquire) == State::EMPTY) {
                task.func = std::move(func);
                task.state.store(State::READY, std::memory_order_release);
                break;
            }
            std::this_thread::yield();
        }
    }

    void barrier() {
        while (pendingTasks.load(std::memory_order_acquire) > 0) {
            std::this_thread::yield();
        }
    }
};

#endif