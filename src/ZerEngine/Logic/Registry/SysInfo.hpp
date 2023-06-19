#ifndef ZERENGINE_LOGIC_REGISTRY_SYS_INFO_HPP
#define ZERENGINE_LOGIC_REGISTRY_SYS_INFO_HPP

#include <cstdint>
#include <cstdlib>
#include <iostream>

#ifdef _WIN32
    #include <sysinfoapi.h>
#elif defined(__APPLE__) || defined(__linux__)
    #include <unistd.h>
#endif

#if defined(__linux__)
    #include <malloc.h>
#elif defined(__APPLE__)
    #include <malloc/malloc.h>
#endif

static inline size_t getPageSize() noexcept {
    #ifdef _WIN32
        SYSTEM_INFO sSysInfo;
        GetSystemInfo(&sSysInfo);
        return sSysInfo.dwPageSize;
    #elif defined(__APPLE__) || defined(__linux__)
        return static_cast<size_t>(getpagesize());
    #else
        return 4096;
    #endif
}

static const size_t pageSize = getPageSize();
/*
#if defined(__APPLE__) || defined(__linux__)
    static inline void* unix_aligned_malloc(size_t align, size_t size) {
        void* ptr = nullptr;
        if (posix_memalign(&ptr, align, size) != 0) {
            std::cerr << "Posix Memmory Alignement Fail" << std::endl;
            exit(1);
        }
        return ptr;
    }

    static inline void* unix_aligned_realloc(void* ptr, size_t size, size_t align) noexcept {
        void* newAlloc = nullptr;
        if (posix_memalign(&newAlloc, align, size) != 0) {
            std::cerr << "Posix Memmory Alignement Fail" << std::endl;
            exit(1);
        }
        #ifdef __linux__
            memcpy(newAlloc, ptr, (size >= malloc_usable_size(ptr)) ? malloc_usable_size(ptr) : size);
        #elif __APPLE__
            memcpy(newAlloc, ptr, (size >= malloc_size(ptr)) ? malloc_size(ptr) : size);
        #endif
        free(ptr);
        return newAlloc;
    }

    static inline void unix_aligned_free(void* ptr) {
        std::free(ptr);
    }
#endif
*/
#ifdef _WIN32
    #define aligned_malloc(size, align) \
        (_aligned_malloc(size, align))
#elif defined(__APPLE__) || defined(__linux__)
    #define aligned_malloc(size, align) \
        (aligned_alloc(align, size))
#else
    #define aligned_malloc(size, align) \
        (malloc(size))
#endif

#ifdef _WIN32
    #define aligned_realloc(ptr, size, align) \
        (_aligned_realloc(ptr, size, align))
#elif defined(__APPLE__) || defined(__linux__)
    #define aligned_realloc(ptr, size, align) \
        (unix_aligned_realloc(ptr, size, align))
#else
    #define aligned_realloc(ptr, size, align) \
        (realloc(ptr, size))
#endif

#ifdef _WIN32
    #define aligned_free(ptr) \
        (_aligned_free(ptr))
#elif defined(__APPLE__) || defined(__linux__)
    #define aligned_free(ptr) \
        (free(ptr))
#else
    #define aligned_free(ptr) \
        (free(ptr))
#endif

#endif /** ZERENGINE_LOGIC_REGISTRY_SYS_INFO_HPP */