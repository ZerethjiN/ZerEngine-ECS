#pragma once

#include "Archetype.hpp"
#include "Query.hpp"
#include "../Systems/Systems.hpp"
#include <queue>
#include <unordered_map>
#include <unordered_set>

using namespace std;

namespace zre::priv {
    class ICompPool {
    public:
        virtual ~ICompPool() noexcept = default;
        virtual constexpr void del(void* ptr) const noexcept = 0;
    };

    template <typename T>
    class CompPool: public ICompPool {
    public:
        constexpr void del(void* ptr) const noexcept override final {
            static_cast<T*>(ptr)->~T();
        }
    };

    class Registry {
    public:
        inline Registry(Sys& newSys) noexcept:
            lastEnt(0),
            sys(newSys) {
        }

        ~Registry() noexcept {
            for (auto& pair: entArch) {
                for (auto& pairComp: pair.second->typeInfos) {
                    pools.at(pairComp.first)->del(pair.second->getEntPtr(pair.first, pairComp.first));
                }
            }
            for (auto& pair: pools) {
                delete pair.second;
            }
            for (auto* arch: archs) {
                delete arch;
            }
        }

        template <typename... Comps>
        [[nodiscard]] inline Ent directNewEnt(Comps&&... comps) noexcept {
            Ent ent = getEntToken();

            if (!entTokens.empty()) {
                lastEnt--;
                ent = entTokens.back();
                entTokens.pop_back();
            }

            if constexpr (sizeof...(Comps) > 0) {
                fillCompPoolRec<Comps...>();
            }

            if (archsBySize.contains(sizeof...(Comps))) {
                for (auto* arch: archsBySize.at(sizeof...(Comps))) {
                    if (compatibleArchetype<Comps...>(arch->typeInfos)) {
                        arch->directNewEnt(ent, std::forward<Comps>(comps)...);
                        entArch.emplace(ent, arch);
                        return ent;
                    }
                }
            } else {
                archsBySize.emplace(sizeof...(Comps), std::unordered_set<Archetype*>());
            }

            auto* arch = new Archetype();
            archs.emplace(arch);
            archsBySize.at(sizeof...(Comps)).emplace(arch);
            arch->directCreate<Comps...>();
            arch->directNewEnt(ent, std::forward<Comps>(comps)...);
            entArch.emplace(ent, arch);
            return ent;
        }

        template <typename T, typename... Ts>
        constexpr void fillCompPoolRec() noexcept {
            if (!pools.contains(typeid(T).hash_code())) {
                pools.emplace(typeid(T).hash_code(), new CompPool<T>());
            }
            if constexpr (sizeof...(Ts) > 0) {
                fillCompPoolRec<Ts...>();
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

        inline void newEnt(const Ent ent, const std::vector<LateUpgradeData>& comps) noexcept {
            if (archsBySize.contains(comps.size())) {
                for (auto* arch: archsBySize.at(comps.size())) {
                    if (compatibleArchetype(arch->typeInfos, comps)) {
                        arch->newEnt(ent, comps);
                        entArch.emplace(ent, arch);
                        return;
                    }
                }
            } else {
                archsBySize.emplace(comps.size(), std::unordered_set<Archetype*>());
            }

            auto* arch = new Archetype();
            archs.emplace(arch);
            archsBySize.at(comps.size()).emplace(arch);
            arch->create(comps);
            arch->newEnt(ent, comps);
            entArch.emplace(ent, arch);
        }

        inline void add(const Ent ent, const vector<LateUpgradeData>& comps) noexcept {
            if (!entArch.contains(ent)) {
                return;
            }

            auto* archOld = entArch.at(ent);

            if (archsBySize.contains(archOld->typeInfos.size() + comps.size())) {
                for (auto* arch: archsBySize.at(archOld->typeInfos.size() + comps.size())) {
                    if (compatibleArchetypeWith(arch->typeInfos, archOld->typeInfos, comps)) {
                        arch->add(ent, *archOld, comps);
                        entArch.at(ent) = arch;
                        if (archOld->size() <= 0 && archOld->typeInfos.size() > 0) {
                            archsBySize.at(archOld->typeInfos.size()).erase(archOld);
                            delete *archs.find(archOld);
                            archs.erase(archOld);
                        }
                        return;
                    }
                }
            } else {
                archsBySize.emplace(archOld->typeInfos.size() + comps.size(), std::unordered_set<Archetype*>());
            }

            auto* arch = new Archetype();
            archs.emplace(arch);
            archsBySize.at(archOld->typeInfos.size() + comps.size()).emplace(arch);
            arch->createWithNewTypes(*archOld, comps);
            arch->add(ent, *archOld, comps);
            entArch.at(ent) = arch;
            if (archOld->size() <= 0 && archOld->typeInfos.size() > 0) {
                archsBySize.at(archOld->typeInfos.size()).erase(archOld);
                delete *archs.find(archOld);
                archs.erase(archOld);
            }
        }

        inline void del(const Ent ent, const std::vector<LateUpgradeDelCompData>& comps) noexcept {
            if (!entArch.contains(ent)) {
                return;
            }

            auto* archOld = entArch.at(ent);
            
            for (auto& comp: comps) {
                pools.at(comp.type)->del(archOld->getEntPtr(ent, comp.type));
            }

            if (archsBySize.contains(archOld->typeInfos.size() - comps.size())) {
                for (auto* arch: archsBySize.at(archOld->typeInfos.size() - comps.size())) {
                    if (compatibleArchetypeWithout(arch->typeInfos, archOld->typeInfos, comps)) {
                        arch->del(ent, *archOld, comps);
                        entArch.at(ent) = arch;
                        if (archOld->size() <= 0 && archOld->typeInfos.size() > 0) {
                            archsBySize.at(archOld->typeInfos.size()).erase(archOld);
                            delete *archs.find(archOld);
                            archs.erase(archOld);
                        }
                        return;
                    }
                }
            } else {
                archsBySize.emplace(archOld->typeInfos.size() - comps.size(), std::unordered_set<Archetype*>());
            }

            auto* arch = new Archetype();
            archs.emplace(arch);
            archsBySize.at(archOld->typeInfos.size() - comps.size()).emplace(arch);
            arch->createWithoutType(*archOld, comps);
            arch->del(ent, *archOld, comps);
            entArch.at(ent) = arch;
            if (archOld->size() <= 0 && archOld->typeInfos.size() > 0) {
                archsBySize.at(archOld->typeInfos.size()).erase(archOld);
                delete *archs.find(archOld);
                archs.erase(archOld);
            }
        }

        constexpr void destroy(const Ent ent) noexcept {
            if (!entArch.contains(ent)) {
                return;
            }

            auto* arch = entArch.at(ent);
            for (auto& pair: arch->typeInfos) {
                pools.at(pair.first)->del(arch->getEntPtr(ent, pair.first));
            }
            arch->destroy(ent);
            if (arch->size() <= 0 && arch->typeInfos.size() > 0) {
                archsBySize.at(arch->typeInfos.size()).erase(arch);
                delete *archs.find(arch);
                archs.erase(arch);
            }
            entArch.erase(ent);
            entTokens.push_back(ent);
        }

        template <typename T>
        [[nodiscard]] constexpr bool contains(const Ent ent) const noexcept {
            if (!entArch.contains(ent)) {
                return false;
            }
            return entArch.at(ent)->typeInfos.contains(typeid(T).hash_code());
        }

        template <typename T>
        [[nodiscard]] constexpr T& get(const Ent ent) noexcept {
            return entArch.at(ent)->getEnt<T>(ent);
        }

        template <typename T>
        [[nodiscard]] constexpr const T& get(const Ent ent) const noexcept {
            return entArch.at(ent)->getEnt<T>(ent);
        }

        template <typename... Comps, typename... Filters, typename... Excludes>
        [[nodiscard]] constexpr const Query<Comps...> query(const With<Filters...>& = {}, const Without<Excludes...>& = {}) noexcept {
            std::vector<Archetype*> archsVec;
            size_t minlength = sizeof...(Comps) + sizeof...(Filters) - sizeof...(Excludes);
            for (auto& pair: archsBySize) {
                if (pair.first >= minlength) {
                    for (auto* arch: pair.second) {
                        if (verifyArchetypeSig(arch->typeInfos, with<Comps..., Filters...>, without<Excludes...>)) {
                            archsVec.push_back(arch);
                        }
                    }
                }
            }
            return Query<Comps...>(archsVec, sys);
        }

        inline void clear() noexcept {
            for (auto* arch: archs) {
                delete arch;
            }
            archs.clear();
            archsBySize.clear();
            entArch.clear();
            lastEnt = 0;
        }

    private:
        template <typename... Args>
        [[nodiscard]] constexpr bool compatibleArchetype(const std::unordered_map<Type, TypeInfo>& sig) const noexcept {
            if constexpr (sizeof...(Args) > 0)
                return compatibleArchetypeRec<Args...>(sig);
            else
                return true;
        }

        template <typename Arg, typename... Args>
        [[nodiscard]] constexpr bool compatibleArchetypeRec(const std::unordered_map<Type, TypeInfo>& sig) const noexcept {
            if (sig.contains(typeid(Arg).hash_code())) {
                if constexpr (sizeof...(Args) > 0)
                    return compatibleArchetypeRec<Args...>(sig);
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
            for (const auto& pair: othSig) {
                bool isFound = false;
                for (const auto& comp: comps) {
                    if (pair.first == comp.type) {
                        isFound = true;
                    }
                }
                if (!isFound) {
                    bool subFind = false;
                    for (const auto& othPair: sig) {
                        if (pair.first == othPair.first) {
                            subFind = true;
                        }
                    }
                    if (!subFind) {
                        return false;
                    }
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
        std::unordered_set<Archetype*> archs;
        std::unordered_map<size_t, std::unordered_set<Archetype*>> archsBySize;
        size_t lastEnt;
        std::vector<Ent> entTokens;
        std::unordered_map<Ent, Archetype*> entArch;
        std::unordered_map<Type, ICompPool*> pools;
        Sys& sys;
    };
}