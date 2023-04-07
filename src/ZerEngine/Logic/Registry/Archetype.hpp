#pragma once

#include "SysInfo.hpp"
#include "Chunk.hpp"
#include <vector>
#include <map>
#include <unordered_map>
#include <bit>

using namespace std;

namespace zre::priv {
    struct LateUpgradeData {
        const Type type;
        const size_t size;
        const void* data;
        const size_t offset;

        constexpr LateUpgradeData(const Type newType, const size_t newSize, const void* newData = nullptr, const size_t newOffset = 0) noexcept:
            type(newType),
            size(newSize),
            data(newData),
            offset(newOffset) {
        }
    };

    struct LateUpgradeDelCompData {
        const Type type;
        const size_t size;

        constexpr LateUpgradeDelCompData(const Type newType, const size_t newSize) noexcept:
            type(newType),
            size(newSize) {
        }
    };

    class Archetype {
    public:
        inline Archetype() noexcept:
            rowSize(0),
            maxCapacity(0) {
        }

        ~Archetype() noexcept {
            for (auto& chunk: chunks) {
                Chunk::dealloc(chunk);
            }
        }

        template <typename... Comps>
        constexpr void directCreate() noexcept {
            typeInfos.reserve(sizeof...(Comps));
            directCreateRec<Comps...>();
            if ((pageSize >= rowSize)) {
                maxCapacity = (pageSize >> static_cast<size_t>(bit_width(rowSize))) - 1;
            }
            shiftMultiplier = static_cast<size_t>(bit_width(maxCapacity));
        }

        template <typename Comp, typename... Comps>
        constexpr void directCreateRec() noexcept {
            typeInfos.emplace(
                typeid(Comp).hash_code(),
                TypeInfo(sizeof(Comp), rowSize)
            );
            rowSize += sizeof(Comp);
            if constexpr (sizeof...(Comps) > 0) {
                directCreateRec<Comps...>();
            }
        }

        constexpr void create(const vector<LateUpgradeData>& comps) noexcept {
            typeInfos.reserve(comps.size());
            for (auto& comp: comps) {
                typeInfos.emplace(comp.type, TypeInfo(comp.size, rowSize));
                rowSize += comp.size;
            }
            if ((pageSize >= rowSize)) {
                maxCapacity = (pageSize >> static_cast<size_t>(bit_width(rowSize))) - 1;
            }
            shiftMultiplier = static_cast<size_t>(bit_width(maxCapacity));
        }

        constexpr void createWithNewTypes(Archetype& oldArch, const vector<LateUpgradeData>& comps) noexcept {
            typeInfos = oldArch.typeInfos;
            rowSize = oldArch.rowSize;
            for (auto& comp: comps) {
                typeInfos.emplace(comp.type, TypeInfo(comp.size, rowSize));
                rowSize += comp.size;
            }
            if ((pageSize >= rowSize)) {
                maxCapacity = (pageSize >> static_cast<size_t>(bit_width(rowSize))) - 1;
            }
            shiftMultiplier = static_cast<size_t>(bit_width(maxCapacity));
        }

        inline void createWithoutType(Archetype& oldArch, const vector<LateUpgradeDelCompData>& comps) noexcept {
            for (auto& pair: oldArch.typeInfos) {
                bool skip = false;
                for (auto& comp: comps) {
                    if (pair.first == comp.type) {
                        skip = true;
                        break;
                    }
                }
                if (!skip) {
                    typeInfos.emplace(pair.first, TypeInfo(pair.second.size, rowSize));
                    rowSize += pair.second.size;
                }
            }
            if ((pageSize >= rowSize)) {
                maxCapacity = (pageSize >> static_cast<size_t>(bit_width(rowSize))) - 1;
            }
            shiftMultiplier = static_cast<size_t>(bit_width(maxCapacity));
        }

        template <typename... Comps>
        constexpr void directNewEnt(Ent ent, const Comps&... comps) noexcept {
            if (!(entIdx.size() & maxCapacity)) {
                void* newChunk;
                if (rowSize << 2 <= pageSize)
                    newChunk = Chunk::alloc();
                else
                    newChunk = Chunk::unboundedAlloc(rowSize);
                chunks.emplace_back(newChunk);
            }
            size_t tmpLength = entIdx.size();
            entIdx.emplace(ent, tmpLength);
            idxEnt.emplace(tmpLength, ent);

            directNewEntRec(chunks.back(), tmpLength, comps...);
        }

        template <typename Comp, typename... Comps>
        constexpr void directNewEntRec(void* curChunk, const size_t tmpLength, const Comp& comp, const Comps&... comps) noexcept {
            Chunk::directSetComp(
                curChunk,
                tmpLength & maxCapacity,
                rowSize,
                comp,
                typeInfos.at(typeid(Comp).hash_code())
            );
            if constexpr (sizeof...(Comps) > 0) {
                directNewEntRec(curChunk, tmpLength, comps...);
            }
        }

        constexpr void newEnt(Ent ent, const vector<LateUpgradeData>& comps) noexcept {
            if (!(entIdx.size() & maxCapacity)) {
                void* newChunk = nullptr;
                if (rowSize << 2 <= pageSize)
                    newChunk = Chunk::alloc();
                else
                    newChunk = Chunk::unboundedAlloc(rowSize);
                chunks.emplace_back(newChunk);
            }
            auto& curChunk = chunks.back();
            size_t tmpLength = entIdx.size();
            entIdx.emplace(ent, tmpLength);
            idxEnt.emplace(tmpLength, ent);

            for (auto& comp: comps) {
                Chunk::setComp(
                    curChunk,
                    tmpLength & maxCapacity,
                    rowSize,
                    static_cast<const byte*>(comp.data) + comp.offset,
                    typeInfos.at(comp.type)
                );
            }
        }

        template <typename T>
        [[nodiscard]] constexpr T& getEnt(Ent ent) noexcept {
            auto idx = entIdx.at(ent);
            return Chunk::get<T>(chunks[idx >> shiftMultiplier], idx & maxCapacity, rowSize, typeInfos.at(typeid(T).hash_code()).offset);
        }

        template <typename T>
        [[nodiscard]] constexpr const T& getEnt(Ent ent) const noexcept {
            auto idx = entIdx.at(ent);
            return Chunk::get<T>(chunks[idx >> shiftMultiplier], idx & maxCapacity, rowSize, typeInfos.at(typeid(T).hash_code()).offset);
        }

        inline void add(Ent ent, Archetype& archOld, const vector<LateUpgradeData>& comps) noexcept {
            if ((entIdx.size() & maxCapacity) == 0) {
                void* newChunk;
                if (rowSize << 2 <= pageSize)
                    newChunk = Chunk::alloc();
                else
                    newChunk = Chunk::unboundedAlloc(rowSize);
                chunks.emplace_back(newChunk);
            }
            auto& curChunk = chunks.back();
            size_t tmpLength = entIdx.size();
            entIdx.emplace(ent, tmpLength);
            idxEnt.emplace(tmpLength, ent);

            for (const auto& pair: archOld.typeInfos) {
                Chunk::copyComp(
                    curChunk,
                    tmpLength & maxCapacity,
                    archOld.entIdx.at(ent) & archOld.maxCapacity,
                    typeInfos.at(pair.first),
                    archOld.typeInfos.at(pair.first),
                    archOld.chunks[archOld.entIdx.at(ent) >> archOld.shiftMultiplier],
                    rowSize,
                    archOld.rowSize
                );
            }

            for (const auto& comp: comps) {
                Chunk::setComp(
                    curChunk,
                    tmpLength & maxCapacity,
                    rowSize,
                    comp.data,
                    typeInfos.at(comp.type)
                );
            }

            archOld.destroy(ent);
        }

        inline void del(Ent ent, Archetype& archOld, const vector<LateUpgradeDelCompData>& comps) noexcept {
            if (!(entIdx.size() & maxCapacity)) {
                void* newChunk;
                if (rowSize << 2 <= pageSize)
                    newChunk = Chunk::alloc();
                else
                    newChunk = Chunk::unboundedAlloc(rowSize);
                chunks.emplace_back(newChunk);
            }
            auto& curChunk = chunks.back();
            size_t tmpLength = entIdx.size();
            entIdx.emplace(ent, tmpLength);
            idxEnt.emplace(tmpLength, ent);

            for (const auto& pair: archOld.typeInfos) {
                bool isValid = true;
                for (const auto& comp: comps) {
                    if (pair.first == comp.type) {
                        isValid = false;
                        break;
                    }
                }
                if (isValid) {
                    Chunk::copyComp(
                        curChunk,
                        tmpLength & maxCapacity,
                        archOld.entIdx.at(ent) & archOld.maxCapacity,
                        typeInfos.at(pair.first),
                        archOld.typeInfos.at(pair.first),
                        archOld.chunks[archOld.entIdx.at(ent) >> archOld.shiftMultiplier],
                        rowSize,
                        archOld.rowSize
                    );
                }
            }

            archOld.destroy(ent);
        }

        inline void destroy(Ent ent) noexcept {
            auto idx = entIdx.at(ent);
            auto& lastChunk = chunks.back();
            size_t tmpLength = entIdx.size() - 1;

            Chunk::copyRow(
                chunks[idx >> shiftMultiplier],
                idx & maxCapacity,
                tmpLength & maxCapacity,
                lastChunk,
                rowSize
            );

            auto alterEnt = idxEnt.at(tmpLength);
            entIdx.at(alterEnt) = idx;
            idxEnt.at(idx) = alterEnt;

            entIdx.erase(ent);
            idxEnt.erase(tmpLength);

            if ((tmpLength & maxCapacity) == 0) {
                Chunk::dealloc(lastChunk);
                chunks.pop_back();
            }
        }

        template <typename Func, typename... Args>
        inline void each(const Func& func) const noexcept {
            for (const auto& pair: entIdx) {
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

        [[nodiscard]] constexpr size_t size() const noexcept {
            return entIdx.size();
        }

    private:
        template <typename T>
        [[nodiscard]] constexpr T& getAt(const size_t idx) const noexcept {
            return Chunk::get<T>(chunks[idx >> shiftMultiplier], idx & maxCapacity, rowSize, typeInfos.at(typeid(T).hash_code()).offset);
        }

    public:
        size_t rowSize;
        size_t maxCapacity;
        size_t shiftMultiplier;
        std::vector<void*> chunks;
        std::unordered_map<Type, TypeInfo> typeInfos;
        std::unordered_map<Ent, size_t> entIdx;
        std::unordered_map<size_t, Ent> idxEnt;
    };
}