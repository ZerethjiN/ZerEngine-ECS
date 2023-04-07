#pragma once

#include "Archetype.hpp"
#include "Query.hpp"
#include "LiteArchetype.hpp"
#include "LiteQuery.hpp"

using namespace std;

namespace zre::priv {
    class LiteRegistry {
    public:
        ~LiteRegistry() noexcept {
            for (auto* arch: archs) {
                delete arch;
            }
        }

        template <typename... Comps, typename... Filters, typename... Excludes>
        [[nodiscard]] inline const LiteQuery<Comps...> query(const With<Filters...>& = {}, const Without<Excludes...>& = {}) noexcept {
            std::vector<LiteArchetype*> archsVec;
            for (auto* arch: archs) {
                if (arch->typeInfos.size() >= sizeof...(Comps) + sizeof...(Filters)) {
                    if (verifyArchetypeSig(arch->typeInfos, with<Comps..., Filters...>, without<Excludes...>)) {
                        archsVec.push_back(arch);
                    }
                }
            }
            return LiteQuery<Comps...>(archsVec);
        }

        template <typename... Comps>
        constexpr void copyFromQuery(const Query<Comps...>& query) noexcept {
            for (const auto* arch: query.archs) {
                archs.emplace_back(new LiteArchetype())->copyArchetype(*arch);
            }
        }

        constexpr void clear() noexcept {
            for (auto* arch: archs) {
                delete arch;
            }
            archs.clear();
        }

    private:
        template <typename Arg, typename... Args>
        [[nodiscard]] constexpr bool compatibleArchetype(const unordered_map<Type, TypeInfo>& sig) const noexcept {
            if (sig.contains(typeid(Arg).hash_code())) {
                if constexpr (sizeof...(Args) > 0)
                    return compatibleArchetype<Args...>(sig);
                else
                    return true;
            }
            return false;
        }

        template <typename... Comps, typename... Excludes>
        [[nodiscard]] constexpr bool verifyArchetypeSig(const unordered_map<Type, TypeInfo>& sig, const With<Comps...>&, const Without<Excludes...>&) const noexcept {
            if (!compatibleArchetype<Comps...>(sig)) {
                return false;
            }
            if constexpr (sizeof...(Excludes) > 0) {
                return excludeCompatibleArchetype<Excludes...>(sig);
            }
            return true;
        }

        template <typename Exclude, typename... Excludes>
        [[nodiscard]] inline bool excludeCompatibleArchetype(const unordered_map<Type, TypeInfo>& sig) const noexcept {
            if (sig.contains(typeid(Exclude).hash_code())) {
                if constexpr (sizeof...(Excludes) > 0)
                    return excludeCompatibleArchetype<Excludes...>(sig);
                else
                    return false;
            }
            return true;
        }

    private:
        std::vector<LiteArchetype*> archs;
    };
}