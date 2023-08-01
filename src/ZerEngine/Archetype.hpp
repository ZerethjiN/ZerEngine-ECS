#pragma once

#include <vector>
#include <unordered_map>
#include <bit>
#include <cstring>
#include "Utils.hpp"
#include "OSDependency.hpp"

class Archetype final {
public:
    inline ~Archetype() noexcept {
        for (auto* data: datas) {
            if (pagesize >= (rowSize << 1)) {
                aligned_free(data);
            } else {
                free(data);
            }
        }
    }

    constexpr void create(const std::vector<LateUpgradeAddData>& comps) noexcept {
        types.reserve(comps.size());
        for (const auto& comp: comps) {
            types.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(comp.type),
                std::forward_as_tuple(comp.size, rowSize)
            );
            rowSize += comp.size;
        }
        if (pagesize >= (rowSize << 1)) {
            maxCapacity = (pagesize >> static_cast<std::size_t>(std::bit_width(rowSize))) -1;
            shiftMultiplier = static_cast<std::size_t>(std::bit_width(maxCapacity));
        }
    }

    constexpr void createWith(const Archetype& oldArch, const LateUpgradeAddData& comp) noexcept {
        types = oldArch.types;
        rowSize = oldArch.rowSize;
        if (!types.contains(comp.type)) {
            types.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(comp.type),
                std::forward_as_tuple(comp.size, rowSize)
            );
            rowSize += comp.size;
        }
        if (pagesize >= (rowSize << 1)) {
            maxCapacity = (pagesize >> static_cast<std::size_t>(std::bit_width(rowSize))) -1;
            shiftMultiplier = static_cast<std::size_t>(std::bit_width(maxCapacity));
        }
    }

    inline void createWithout(const Archetype& oldArch, const LateUpgradeDelCompData& comp) noexcept {
        for (const auto& pairTypes: oldArch.types) {
            if (pairTypes.first != comp.type) {
                types.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(pairTypes.first),
                    std::forward_as_tuple(pairTypes.second.size, rowSize)
                );
                rowSize += pairTypes.second.size;
            }
        }
        if (pagesize >= (rowSize << 1)) {
            maxCapacity = (pagesize >> static_cast<std::size_t>(std::bit_width(rowSize))) -1;
            shiftMultiplier = static_cast<std::size_t>(std::bit_width(maxCapacity));
        }
    }

    constexpr void newEnt(const Ent ent, const std::vector<LateUpgradeAddData>& comps) noexcept {
        if (!(entIdx.size() & maxCapacity)) {
            if (pagesize >= (rowSize << 1)) {
                datas.emplace_back(aligned_malloc(pagesize, pagesize));
            } else {
                datas.emplace_back(malloc(rowSize));
            }
        }
        const auto idx = entIdx.size();
        entIdx.emplace(ent, idx);
        idxEnt.emplace(idx, ent);
        auto* data = static_cast<std::byte*>(datas[idx >> shiftMultiplier]) + (rowSize * (idx & maxCapacity));
        for (const auto& comp: comps) {
            memcpy(
                data + types.at(comp.type).offset,
                comp.data,
                comp.size
            );
        }
    }

    inline void add(const Ent ent, Archetype& oldArch, const LateUpgradeAddData& comp) noexcept {
        if (!(entIdx.size() & maxCapacity)) {
            if (pagesize >= (rowSize << 1)) {
                datas.emplace_back(aligned_malloc(pagesize, pagesize));
            } else {
                datas.emplace_back(malloc(rowSize));
            }
        }
        const auto idx = entIdx.size();
        entIdx.emplace(ent, idx);
        idxEnt.emplace(idx, ent);

        auto* data = static_cast<std::byte*>(datas[idx >> shiftMultiplier]) + (rowSize * (idx & maxCapacity));

        const auto othIdx = oldArch.entIdx.at(ent);
        const auto* othData = static_cast<const std::byte*>(oldArch.datas[othIdx >> oldArch.shiftMultiplier]) + (oldArch.rowSize * (othIdx & oldArch.maxCapacity));

        for (const auto& pairTypes: oldArch.types) {
            memcpy(
                data + types.at(pairTypes.first).offset,
                othData + pairTypes.second.offset,
                types.at(pairTypes.first).size
            );
        }

        memcpy(
            data + types.at(comp.type).offset,
            comp.data,
            comp.size
        );

        oldArch.destroy(ent);
    }

    inline void del(const Ent ent, Archetype& oldArch, const LateUpgradeDelCompData& comp) noexcept {
        if (!(entIdx.size() & maxCapacity)) {
            if (pagesize >= (rowSize << 1)) {
                datas.emplace_back(aligned_malloc(pagesize, pagesize));
            } else {
                datas.emplace_back(malloc(rowSize));
            }
        }
        const auto idx = entIdx.size();
        entIdx.emplace(ent, idx);
        idxEnt.emplace(idx, ent);

        auto* data = static_cast<std::byte*>(datas[idx >> shiftMultiplier]) + (rowSize * (idx & maxCapacity));

        const auto othIdx = oldArch.entIdx.at(ent);
        const auto* othData = static_cast<const std::byte*>(oldArch.datas[othIdx >> oldArch.shiftMultiplier]) + (oldArch.rowSize * (othIdx & oldArch.maxCapacity));

        for (const auto& pairTypes: oldArch.types) {
            if (pairTypes.first != comp.type) {
                memcpy(
                    data + types.at(pairTypes.first).offset,
                    othData + pairTypes.second.offset,
                    types.at(pairTypes.first).size
                );
            }
        }

        oldArch.destroy(ent);
    }

    template <typename T>
    [[nodiscard]] constexpr T& get(const Ent ent) noexcept {
        return static_cast<T&>(static_cast<std::byte*>(datas[entIdx.at(ent) >> shiftMultiplier])[(rowSize * (entIdx.at(ent) & maxCapacity)) + types.at(typeid(T).hash_code()).offset]);
    }

    template <typename T>
    [[nodiscard]] constexpr const T& get(const Ent ent) const noexcept {
        return static_cast<T&>(static_cast<std::byte*>(datas[entIdx.at(ent) >> shiftMultiplier])[(rowSize * (entIdx.at(ent) & maxCapacity)) + types.at(typeid(T).hash_code()).offset]);
    }

    [[nodiscard]] constexpr void* getPtr(const Ent ent, const Type type) const noexcept {
        return static_cast<void*>(static_cast<std::byte*>(datas[entIdx.at(ent) >> shiftMultiplier]) + (rowSize * (entIdx.at(ent) & maxCapacity)) + types.at(type).offset);
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return entIdx.size();
    }

    [[nodiscard]] constexpr bool isTotalyCompatibleLate(const std::vector<LateUpgradeAddData>& comps) const noexcept {
        if (comps.size() != types.size()) {
            return false;
        }
        for (const auto& comp: comps) {
            if (!types.contains(comp.type)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] inline bool isTotalyCompatibleLate(const Archetype& oldArch, const LateUpgradeAddData& comp) const noexcept {
        if (oldArch.types.size() + 1 != types.size()) {
            return false;
        }
        if (!types.contains(comp.type)) {
            return false;
        }
        for (const auto& pair: oldArch.types) {
            if (!types.contains(pair.first)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] inline bool isTotalyCompatibleWithoutLate(const Archetype& oldArch, const LateUpgradeDelCompData& comp) const noexcept {
        if (oldArch.types.size() - 1 != types.size() || types.contains(comp.type)) {
            return false;
        }
        for (const auto& pair: oldArch.types) {
            if (!types.contains(pair.first) && pair.first != comp.type) {
                return false;
            }
        }
        return true;
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    [[nodiscard]] constexpr bool isPartialyCompatible(const With<Filters...>& with = {}, const Without<Excludes...>& without = {}) const noexcept {
        if constexpr (sizeof...(Comps) + sizeof...(Filters) > 0) {
            if (!isCompatibleRec<Comps..., Filters...>()) {
                return false;
            }
        }
        if constexpr (sizeof...(Excludes) > 0) {
            if (!isExcludeRec<Excludes...>()) {
                return false;
            }
        }
        return true;
    }

    inline void destroy(const Ent ent) noexcept {
        const auto oldIdx = entIdx.at(ent);
        const auto lastIdx = entIdx.size() - 1;
        const auto lastEnt = idxEnt.at(lastIdx);
        memcpy(
            static_cast<std::byte*>(datas[oldIdx >> shiftMultiplier]) + (rowSize * (oldIdx & maxCapacity)),
            static_cast<const std::byte*>(datas[lastIdx >> shiftMultiplier]) + (rowSize * (lastIdx & maxCapacity)),
            rowSize
        );
        entIdx.at(lastEnt) = oldIdx;
        idxEnt.at(oldIdx) = lastEnt;

        entIdx.erase(ent);
        idxEnt.erase(lastIdx);

        if (!(entIdx.size() & maxCapacity)) {
            if (pagesize >= (rowSize << 1)) {
                aligned_free(datas.back());
                datas.pop_back();
            } else {
                free(datas.back());
                datas.pop_back();
            }
        }
    }

    template <typename Func, typename... Ts>
    inline void each(const Func& func) const noexcept {
        if constexpr (std::is_invocable_v<Func, Ts&...>) {
            for (const auto& pair: entIdx) {
                std::apply(func, std::forward_as_tuple(getAt<Ts>(pair.second)...));
            }
        } else if constexpr (std::is_invocable_v<Func, Ent, Ts&...>) {
            for (const auto& pair: entIdx) {
                std::apply(func, std::forward_as_tuple(pair.first, getAt<Ts>(pair.second)...));
            }
        }
    }

private:
    template <typename... Ts>
    [[nodiscard]] constexpr bool isCompatibleRec() const noexcept {
        return (types.contains(typeid(Ts).hash_code()) && ...);
    }

    template <typename... Ts>
    [[nodiscard]] constexpr bool isExcludeRec() const noexcept {
        return (!types.contains(typeid(Ts).hash_code()) && ...);
    }

    template <typename T>
    [[nodiscard]] constexpr T& getAt(const std::size_t idx) const noexcept {
        return reinterpret_cast<T&>(static_cast<std::byte*>(datas[idx >> shiftMultiplier])[(rowSize * (idx & maxCapacity)) + types.at(typeid(T).hash_code()).offset]);
    }

public:
    std::size_t rowSize = 0;
    std::size_t maxCapacity = 0;
    std::size_t shiftMultiplier = 0;
    std::unordered_map<Type, TypeInfos> types;
    std::unordered_map<Ent, std::size_t> entIdx;
    std::unordered_map<std::size_t, Ent> idxEnt;
    std::vector<void*> datas;
};