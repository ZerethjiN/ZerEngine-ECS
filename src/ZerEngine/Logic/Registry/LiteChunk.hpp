#pragma once

#include <cstdlib>
#include <cstring>
#include <cstddef>

#include "SysInfo.hpp"
#include "ArchetypeTypeSizeInfo.hpp"
#include "Chunk.hpp"

using namespace std;

namespace zre::priv {
    class LiteChunk {
    public:
        [[nodiscard]] static inline void* multiAlloc(size_t chunkSize, size_t nbChunks) noexcept {
            return malloc(chunkSize * nbChunks);
        }

        template <typename T>
        [[nodiscard]] static constexpr T& get(void* datas, size_t idx, size_t rowSize, size_t offset) noexcept {
            return reinterpret_cast<T&>(static_cast<byte*>(datas)[rowSize * idx + offset]);
        }

        static inline void copyChunk(void* datas, size_t chunkIdx, size_t chunkSize, void* othDatas) noexcept {
            memcpy(
                static_cast<byte*>(datas) + (chunkSize * chunkIdx),
                othDatas,
                chunkSize
            );
        }

        static void dealloc(void* datas) noexcept {
            free(datas);
        }
    };
}