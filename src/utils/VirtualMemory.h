#ifndef VIRTUALMEMORY_H_INCLUDED
#define VIRTUALMEMORY_H_INCLUDED

#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#else
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace VirtualMemory {
    struct MemoryInfo {
        size_t pageSize;
        size_t allocationGranularity;
    };

    inline MemoryInfo memoryInfo() {
        MemoryInfo memInfo;
#ifdef _WIN32
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        memInfo.pageSize = static_cast<size_t>(info.dwPageSize);
        memInfo.allocationGranularity = static_cast<size_t>(info.dwAllocationGranularity);
#else
        memInfo.pageSize = static_cast<size_t>(sysconf(_SC_PAGESIZE));
        memInfo.allocationGranularity = memInfo.pageSize;
#endif
        return memInfo;
    }

    inline void* reserve(size_t size) {
#ifdef _WIN32
        return VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS);
#else
        int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#ifdef MAP_NORESERVE
        flags |= MAP_NORESERVE;
#endif
        void* p = mmap(nullptr, size, PROT_NONE, flags, -1, 0);
        return (p == MAP_FAILED) ? nullptr : p;
#endif
    }

    inline bool commit(void* ptr, size_t size) {
#ifdef _WIN32
        return VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != nullptr;
#else
        return mprotect(ptr, size, PROT_READ | PROT_WRITE) == 0;
#endif
    }

    inline void decommit(void* ptr, size_t size) {
#ifdef _WIN32
        VirtualFree(ptr, size, MEM_DECOMMIT);
#else
        madvise(ptr, size, MADV_DONTNEED);
        mprotect(ptr, size, PROT_NONE);
#endif
    }

    inline void release(void* ptr, size_t size) {
#ifdef _WIN32
        VirtualFree(ptr, 0, MEM_RELEASE);
#else
        munmap(ptr, size);
#endif
    }
}

#endif