#pragma once

#include <cstdlib>
#include <cstring>
#include <cstddef>

#include "SysInfo.hpp"
#include "ArchetypeTypeSizeInfo.hpp"

using namespace std;

namespace zre::priv {
    class Chunk {
    public:
        [[nodiscard]] static inline void* alloc() noexcept {
            return aligned_malloc(pageSize, pageSize);
        }

        [[nodiscard]] static constexpr void* unboundedAlloc(const size_t rowSize) noexcept {
            return malloc(rowSize);
        }

        template <typename T>
        [[nodiscard]] static constexpr T& get(void* datas, const size_t idx, const size_t rowSize, const size_t offset) noexcept {
            return reinterpret_cast<T&>(static_cast<byte*>(datas)[rowSize * idx + offset]);
        }

        [[nodiscard]] static constexpr void* getPtr(void* datas, const size_t idx, const size_t rowSize, const size_t offset) noexcept {
            return static_cast<void*>(static_cast<byte*>(datas) + (rowSize * idx) + offset);
        }

        template <typename Comp>
        static inline void directSetComp(void* datas, const size_t idx, const size_t rowSize, Comp&& newData, const TypeInfo& info) noexcept {
            memcpy(
                static_cast<byte*>(datas) + (rowSize * idx) + info.offset,
                &newData,
                info.size
            );
        }

        static inline void setComp(void* datas, const size_t idx, const size_t rowSize, const void* newData, const TypeInfo& info) noexcept {
            memcpy(
                static_cast<byte*>(datas) + (rowSize * idx) + info.offset,
                newData,
                info.size
            );
        }

        static inline void copyComp(void* datas, const size_t idx, const size_t othIdx, const TypeInfo& info, const TypeInfo& othInfo, const void* othDatas, size_t rowSize, size_t othRowSize) noexcept {
            memcpy(
                static_cast<byte*>(datas) + (rowSize * idx) + info.offset,
                static_cast<const byte*>(othDatas) + (othRowSize * othIdx) + othInfo.offset,
                info.size
            );
        }

        static inline void copyRow(void* datas, const size_t idx, const size_t othIdx, const void* othDatas, size_t rowSize) noexcept {
            memcpy(
                static_cast<byte*>(datas) + (rowSize * idx),
                static_cast<const byte*>(othDatas) + (rowSize * othIdx),
                rowSize
            );
        }

        static inline void copyChunk(void* datas, const void* othDatas) noexcept {
            memcpy(
                datas,
                othDatas,
                pageSize
            );
        }

        static void dealloc(void* datas) noexcept {
            aligned_free(datas);
        }
    };
}