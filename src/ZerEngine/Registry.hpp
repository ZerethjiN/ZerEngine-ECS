#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "Utils.hpp"
#include "Archetype.hpp"
#include "View.hpp"

class Registry final {
public:
    inline ~Registry() noexcept {
        for (auto& pair: entArch) {
            for (const auto& pairType: pair.second->types) {
                destructors.at(pairType.first)->del(pair.second->getPtr(pair.first, pairType.first));
            }
        }
        for (auto& pair: destructors) {
            delete pair.second;
        }
        for (auto* arch: archs) {
            delete arch;
        }
    }

    [[nodiscard]] constexpr Ent getEntToken() noexcept {
        Ent ent = lastEnt++;

        if (!entTokens.empty()) {
            lastEnt--;
            ent = entTokens.back();
            entTokens.pop_back();
        }

        return ent;
    }

    inline void newEnt(const Ent ent, const std::vector<LateUpgradeAddData>& comps) noexcept {
        if (archsBySize.contains(comps.size())) {
            for (auto* arch: archsBySize.at(comps.size())) {
                if (arch->isTotalyCompatibleLate(comps)) {
                    arch->newEnt(ent, comps);
                    entArch.emplace(ent, arch);
                    return;
                }
            }
        } else {
            archsBySize.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(comps.size()),
                std::forward_as_tuple(std::unordered_set<Archetype*>())
            );
        }

        auto* arch = new Archetype();
        archs.emplace(arch);
        archsBySize.at(comps.size()).emplace(arch);
        arch->create(comps);
        arch->newEnt(ent, comps);
        entArch.emplace(ent, arch);
        return;
    }

    template <typename T>
    [[nodiscard]] constexpr T& get(const Ent ent) noexcept {
        return entArch.at(ent)->get<T>(ent);
    }

    template <typename T>
    [[nodiscard]] constexpr const T& get(const Ent ent) const noexcept {
        return entArch.at(ent)->get<T>(ent);
    } 

    template <typename T>
    [[nodiscard]] constexpr bool has(const Ent ent) const noexcept {
        return entArch.at(ent)->types.contains(typeid(T).hash_code());
    }

    inline void add(const Ent ent, const LateUpgradeAddData& comp) noexcept {
        auto* oldArch = entArch.at(ent);

        if (archsBySize.contains(oldArch->types.size() + 1)) {
            for (auto* arch: archsBySize.at(oldArch->types.size() + 1)) {
                if (arch->isTotalyCompatibleLate(*oldArch, comp)) {
                    arch->add(ent, *oldArch, comp);
                    entArch.at(ent) = arch;
                    if (oldArch->size() <= 0 && oldArch->types.size() > 0) {
                        archsBySize.at(oldArch->types.size()).erase(oldArch);
                        delete *archs.find(oldArch);
                        archs.erase(oldArch);
                    }
                    return;
                }
            }
        } else {
            archsBySize.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(oldArch->types.size() + 1),
                std::forward_as_tuple(std::unordered_set<Archetype*>())
            );
        }

        auto* arch = new Archetype();
        archs.emplace(arch);
        archsBySize.at(oldArch->types.size() + 1).emplace(arch);
        arch->createWith(*oldArch, comp);
        arch->add(ent, *oldArch, comp);
        entArch.at(ent) = arch;
        if (oldArch->size() <= 0 && oldArch->types.size() > 0) {
            archsBySize.at(oldArch->types.size()).erase(oldArch);
            delete *archs.find(oldArch);
            archs.erase(oldArch);
        }
    }

    inline void del(const Ent ent, const LateUpgradeDelCompData& comp) noexcept {
        auto* oldArch = entArch.at(ent);

        destructors.at(comp.type)->del(entArch.at(ent)->getPtr(ent, comp.type));

        if (archsBySize.contains(oldArch->types.size() - 1)) {
            for (auto* arch: archsBySize.at(oldArch->types.size() - 1)) {
                if (arch->isTotalyCompatibleWithoutLate(*oldArch, comp)) {
                    arch->del(ent, *oldArch, comp);
                    entArch.at(ent) = arch;
                    if (oldArch->size() <= 0 && oldArch->types.size() > 0) {
                        archsBySize.at(oldArch->types.size()).erase(oldArch);
                        delete *archs.find(oldArch);
                        archs.erase(oldArch);
                    }
                    return;
                }
            }
        } else {
            archsBySize.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(oldArch->types.size() - 1),
                std::forward_as_tuple(std::unordered_set<Archetype*>())
            );
        }

        auto* arch = new Archetype();
        archs.emplace(arch);
        archsBySize.at(oldArch->types.size() - 1).emplace(arch);
        arch->createWithout(*oldArch, comp);
        arch->del(ent, *oldArch, comp);
        entArch.at(ent) = arch;
        if (oldArch->size() <= 0 && oldArch->types.size() > 0) {
            archsBySize.at(oldArch->types.size()).erase(oldArch);
            delete *archs.find(oldArch);
            archs.erase(oldArch);
        }
    }

    inline void destroy(const Ent ent) noexcept {
        auto* arch = entArch.at(ent);
        for (const auto& pairType: arch->types) {
            destructors.at(pairType.first)->del(arch->getPtr(ent, pairType.first));
        }
        arch->destroy(ent);
        if (arch->size() <= 0 && arch->types.size() > 0) {
            archsBySize.at(arch->types.size()).erase(arch);
            delete *archs.find(arch);
            archs.erase(arch);
        }
        entArch.erase(ent);
        entTokens.push_back(ent);
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    [[nodiscard]] inline const View<Comps...> view(const With<Filters...>& with = {}, const Without<Excludes...>& without = {}) const noexcept {
        std::vector<const Archetype*> viewArchs;
        const constexpr std::size_t minlength = sizeof...(Comps) + sizeof...(Filters) - sizeof...(Excludes);
        for (auto& pair: archsBySize) {
            if (pair.first >= minlength) {
                for (auto* arch: pair.second) {
                    if (arch->isPartialyCompatible<Comps...>(with, without)) {
                        viewArchs.push_back(arch);
                    }
                }
            }
        }
        return {viewArchs};
    }

public:
    template <typename T, typename... Ts>
    constexpr void fillDestructorsRec() noexcept {
        if (!destructors.contains(typeid(T).hash_code())) {
            destructors.emplace(typeid(T).hash_code(), new CompDestructor<T>());
        }
        if constexpr (sizeof...(Ts) > 0) {
            fillDestructorsRec<Ts...>();
        }
    }

private:
    Ent lastEnt = 0;
    std::vector<Ent> entTokens;
    std::unordered_set<Archetype*> archs;
    std::unordered_map<std::size_t, std::unordered_set<Archetype*>> archsBySize;
    std::unordered_map<Ent, Archetype*> entArch;
    std::unordered_map<Type, ICompDestructor*> destructors;
};