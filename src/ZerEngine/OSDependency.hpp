#pragma once

#include <cstdint>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
#elif defined(__APPLE__) || defined(__linux__)
    #include <unistd.h>
#endif

#if defined(__linux__)
    #include <malloc.h>
    #include <cstring>
#elif defined(__APPLE__)
    #include <malloc/malloc.h>
    #include <cstring>
#endif

[[nodiscard]] static inline std::size_t getPageSize() noexcept {
    #ifdef _WIN32
        SYSTEM_INFO sSysInfo;
        GetSystemInfo(&sSysInfo);
        return sSysInfo.dwPageSize;
    #elif defined(__APPLE__) || defined(__linux__)
        return static_cast<std::size_t>(getpagesize());
    #else
        return 4096;
    #endif
}

static const std::size_t pagesize = getPageSize();

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
    #define aligned_free(ptr) \
        (_aligned_free(ptr))
#elif defined(__APPLE__) || defined(__linux__)
    #define aligned_free(ptr) \
        (free(ptr))
#else
    #define aligned_free(ptr) \
        (free(ptr))
#endif