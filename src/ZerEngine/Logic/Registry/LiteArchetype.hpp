#pragma once

#include "SysInfo.hpp"
#include "LiteChunk.hpp"
#include <vector>
#include <map>
#include <unordered_map>
#include "Archetype.hpp"

using namespace std;

namespace zre::priv {
    class LiteArchetype {
    public:
        ~LiteArchetype() {
            LiteChunk::dealloc(bigChunk);
        }

        template <typename... Args, typename Func>
        inline void each(const Func& func) const noexcept {
            for (auto& pair: entIdx) {
                if constexpr (std::is_invocable_v<Func, Args&...>)
                    std::apply(func, std::make_tuple(std::ref(getAt<Args>(pair.second))...));
                else if constexpr (std::is_invocable_v<Func, Ent, Args&...>)
                    std::apply(func, std::make_tuple(pair.first, std::ref(getAt<Args>(pair.second))...));
                else if constexpr (std::is_invocable_v<Func, Ent>)
                    std::apply(func, std::make_tuple(pair.first));
                else
                    func();
            }
        }

        inline void copyArchetype(const Archetype& oth) noexcept {
            size_t chunkSize = oth.rowSize << oth.shiftMultiplier;
            bigChunk = LiteChunk::multiAlloc(chunkSize, oth.chunks.size());
            size_t i = 0;
            for (auto& othChunk: oth.chunks) {
                LiteChunk::copyChunk(bigChunk, i, chunkSize, othChunk);
                i++;
            }
            rowSize = oth.rowSize;
            typeInfos = oth.typeInfos;
            entIdx = oth.entIdx;
        }

        [[nodiscard]] constexpr size_t size() const noexcept {
            return entIdx.size();
        }

    private:
        template <typename T>
        [[nodiscard]] constexpr T& getAt(const size_t idx) const noexcept {
            return LiteChunk::get<T>(bigChunk, idx, rowSize, typeInfos.at(typeid(T).hash_code()).offset);
        }

    public:
        size_t rowSize;
        void* bigChunk;
        unordered_map<Type, TypeInfo> typeInfos;
        unordered_map<Ent, size_t> entIdx;
    };
}