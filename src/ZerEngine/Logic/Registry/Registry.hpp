#pragma once

#include "Archetype.hpp"
#include "Query.hpp"
#include <queue>
#include <unordered_map>

using namespace std;

namespace zre::priv {
    class Registry {
    public:
        inline Registry() noexcept:
            lastEnt(0) {
        }

        ~Registry() noexcept {
            for (auto* arch: archs) {
                delete arch;
            }
        }

        template <typename... Comps>
        [[nodiscard]] constexpr Ent directNewEnt(const Comps&... comps) noexcept {
            Ent ent = 0;

            if (entTokens.empty()) {
                ent = lastEnt++;
            } else {
                ent = entTokens.back();
                entTokens.pop_back();
            }

            for (auto* arch: archs) {
                if (arch->typeInfos.size() == sizeof...(Comps) && compatibleArchetype<Comps...>(arch->typeInfos)) {
                    arch->directNewEnt(ent, comps...);
                    entArch.emplace(ent, arch);
                    return ent;
                }
            }

            auto* arch = archs.emplace_back(new Archetype());
            arch->directCreate<Comps...>();
            arch->directNewEnt(ent, comps...);
            entArch.emplace(ent, arch);
            return ent;
        }

        constexpr void newEnt(const std::vector<LateUpgradeData>& comps) noexcept {
            Ent ent = 0;

            if (entTokens.empty()) {
                ent = lastEnt++;
            } else {
                ent = entTokens.back();
                entTokens.pop_back();
            }

            for (auto* arch: archs) {
                if (arch->typeInfos.size() == comps.size() && compatibleArchetype(arch->typeInfos, comps)) {
                    arch->newEnt(ent, comps);
                    entArch.emplace(ent, arch);
                    return;
                }
            }

            auto* arch = archs.emplace_back(new Archetype());
            arch->create(comps);
            arch->newEnt(ent, comps);
            entArch.emplace(ent, arch);
        }

        constexpr void add(const Ent ent, const vector<LateUpgradeData>& comps) noexcept {
            if (!entArch.contains(ent)) {
                return;
            }

            auto* archOld = entArch.at(ent);

            for (auto* arch: archs) {
                if (arch->typeInfos.size() == archOld->typeInfos.size() + comps.size() && compatibleArchetypeWith(arch->typeInfos, archOld->typeInfos, comps)) {
                    arch->add(ent, *archOld, comps);
                    entArch.at(ent) = arch;
                    if (archOld->size() <= 0 && archOld->typeInfos.size() > 0) {
                        long i = 0;
                        for (auto* delArch: archs) {
                            if (delArch == archOld) {
                                delete delArch;
                                archs.erase(archs.begin() + i);
                                return;
                            }
                            i++;
                        }
                    }
                    return;
                }
            }

            auto* arch = archs.emplace_back(new Archetype());
            arch->createWithNewTypes(*archOld, comps);
            arch->add(ent, *archOld, comps);
            entArch.at(ent) = arch;
            if (archOld->size() <= 0 && archOld->typeInfos.size() > 0) {
                long i = 0;
                for (auto* delArch: archs) {
                    if (delArch == archOld) {
                        delete delArch;
                        archs.erase(archs.begin() + i);
                        break;
                    }
                    i++;
                }
            }
        }

        constexpr void del(const Ent ent, const std::vector<LateUpgradeDelCompData>& comps) noexcept {
            if (!entArch.contains(ent)) {
                return;
            }

            auto* archOld = entArch.at(ent);

            for (auto* arch: archs) {
                if (arch->typeInfos.size() == archOld->typeInfos.size() - 1 && compatibleArchetypeWithout(arch->typeInfos, archOld->typeInfos, comps)) {
                    arch->del(ent, *archOld, comps);
                    entArch.at(ent) = arch;
                    if (archOld->size() <= 0 && archOld->typeInfos.size() > 0) {
                        long i = 0;
                        for (auto* delArch: archs) {
                            if (delArch == archOld) {
                                delete delArch;
                                archs.erase(archs.begin() + i);
                                break;
                            }
                            i++;
                        }
                    }
                    return;
                }
            }

            archs.push_back(new Archetype());
            auto* arch = archs.back();
            arch->createWithoutType(*archOld, comps);
            arch->del(ent, *archOld, comps);
            entArch.at(ent) = arch;
            if (archOld->size() <= 0 && archOld->typeInfos.size() > 0) {
                long i = 0;
                for (auto* delArch: archs) {
                    if (delArch == archOld) {
                        delete delArch;
                        archs.erase(archs.begin() + i);
                        break;
                    }
                    i++;
                }
            }
        }

        constexpr void destroy(const Ent id) noexcept {
            if (!entArch.contains(id)) {
                return;
            }

            auto* arch = entArch.at(id);
            arch->destroy(id);
            if (arch->size() <= 0 && arch->typeInfos.size() > 0) {
                long i = 0;
                for (auto* delArch: archs) {
                    if (delArch == arch) {
                        delete delArch;
                        archs.erase(archs.begin() + i);
                        break;
                    }
                    i++;
                }
            }
            entArch.erase(id);
            entTokens.push_back(id);
        }

        template <typename T>
        [[nodiscard]] constexpr bool contains(const Ent id) const noexcept {
            if (!entArch.contains(id)) {
                return false;
            }
            return entArch.at(id)->typeInfos.contains(typeid(T).hash_code());
        }

        template <typename T>
        [[nodiscard]] constexpr T& get(const Ent id) noexcept {
            return entArch.at(id)->getEnt<T>(id);
        }

        template <typename T>
        [[nodiscard]] constexpr const T& get(const Ent id) const noexcept {
            return entArch.at(id)->getEnt<T>(id);
        }

        template <typename... Comps, typename... Filters, typename... Excludes>
        [[nodiscard]] constexpr const Query<Comps...> query(const With<Filters...>& = {}, const Without<Excludes...>& = {}) noexcept {
            std::vector<Archetype*> archsVec;
            for (auto* arch: archs) {
                if (arch->typeInfos.size() >= sizeof...(Comps) + sizeof...(Filters)) {
                    if (verifyArchetypeSig(arch->typeInfos, with<Comps..., Filters...>, without<Excludes...>)) {
                        archsVec.push_back(arch);
                    }
                }
            }
            return Query<Comps...>(archsVec);
        }

        constexpr void clear() noexcept {
            for (auto* arch: archs) {
                delete arch;
            }
            archs.clear();
            entArch.clear();
            lastEnt = 1;
        }

    private:
        template <typename Arg, typename... Args>
        [[nodiscard]] constexpr bool compatibleArchetype(const std::unordered_map<Type, TypeInfo>& sig) const noexcept {
            if (sig.contains(typeid(Arg).hash_code())) {
                if constexpr (sizeof...(Args) > 0)
                    return compatibleArchetype<Args...>(sig);
                else
                    return true;
            }
            return false;
        }

        [[nodiscard]] constexpr bool compatibleArchetype(const std::unordered_map<Type, TypeInfo>& sig, const std::vector<LateUpgradeData>& comps) const noexcept {
            for (const auto& comp: comps) {
                if (!sig.contains(comp.type)) {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] inline bool compatibleArchetypeWith(const std::unordered_map<Type, TypeInfo>& sig, const std::unordered_map<Type, TypeInfo>& othSig, const std::vector<LateUpgradeData>& comps) const noexcept {
            for (const auto& comp: comps) {
                if (!sig.contains(comp.type)) {
                    return false;
                }
            }
            for (const auto& othPair: othSig) {
                if (!sig.contains(othPair.first)) {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] inline bool compatibleArchetypeWithout(const std::unordered_map<Type, TypeInfo>& sig, const std::unordered_map<Type, TypeInfo>& othSig, const std::vector<LateUpgradeDelCompData>& comps) const noexcept {
            for (const auto& comp: comps) {
                if (sig.contains(comp.type)) {
                    return false;
                }
            }
            for (const auto& othPair: othSig) {
                if (!sig.contains(othPair.first)) {
                    return false;
                }
            }
            return true;
        }

        template <typename... Comps, typename... Excludes>
        [[nodiscard]] constexpr bool verifyArchetypeSig(const std::unordered_map<Type, TypeInfo>& sig, const With<Comps...>&, const Without<Excludes...>&) const noexcept {
            if (!compatibleArchetype<Comps...>(sig)) {
                return false;
            }
            if constexpr (sizeof...(Excludes) > 0) {
                return excludeCompatibleArchetype<Excludes...>(sig);
            }
            return true;
        }

        template <typename Exclude, typename... Excludes>
        [[nodiscard]] constexpr bool excludeCompatibleArchetype(const std::unordered_map<Type, TypeInfo>& sig) const noexcept {
            if (sig.contains(typeid(Exclude).hash_code())) {
                if constexpr (sizeof...(Excludes) > 0)
                    return excludeCompatibleArchetype<Excludes...>(sig);
                else
                    return false;
            }
            return true;
        }

    public:
        std::vector<Archetype*> archs;
        size_t lastEnt;
        std::vector<Ent> entTokens;
        std::unordered_map<Ent, Archetype*> entArch;
    };
}