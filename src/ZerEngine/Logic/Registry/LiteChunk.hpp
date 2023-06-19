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
        [[nodiscard]] static constexpr void* multiAlloc(const size_t chunkSize, const size_t nbChunks) noexcept {
            return malloc(chunkSize * nbChunks);
        }

        template <typename T>
        [[nodiscard]] static constexpr T& get(void* datas, const size_t idx, const size_t rowSize, const size_t offset) noexcept {
            return reinterpret_cast<T&>(static_cast<byte*>(datas)[rowSize * idx + offset]);
        }

        static inline void copyChunk(void* datas, const size_t chunkIdx, const size_t chunkSize, const void* othDatas) noexcept {
            memcpy(
                static_cast<byte*>(datas) + (chunkSize * chunkIdx),
                othDatas,
                chunkSize
            );
        }

        static constexpr void dealloc(void* datas) noexcept {
            free(datas);
        }
    };
}