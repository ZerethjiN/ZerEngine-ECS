/**
 * @file Query.hpp
 * @author ZerethjiN
 * @brief Get and iterate queries by necessary Component,
 * Filter and Exclusion Type.
 * @version 0.1
 * @date 2022-02-22
 * 
 * @copyright Copyright (c) 2022 - ZerethjiN
 * 
 */
#ifndef ZERENGINE_VIEW_HPP
#define ZERENGINE_VIEW_HPP

#include <vector>
#include <algorithm>
#include <functional>
#include "TypeUtilities.hpp"
#include "CompPool.hpp"

namespace zre {
    template <typename, typename, typename, typename>
    class Query;

    template <typename Comp, typename... Comps, typename... Filters, typename... Excludes>
    class Query<Comp, priv::comp_t<Comps...>, priv::With<Filters...>, priv::Without<Excludes...>> {
    public:
        constexpr Query(priv::CompPool<Comp>& comp, priv::CompPool<Comps>&... comps, priv::CompPool<Filters>&... fltrs, priv::CompPool<Excludes>&... excls) noexcept:
            pools(comp, comps...) {
            genEnts(comp);
            if constexpr (sizeof...(comps) > 0)
                intersectRec(comps...);
            if constexpr (sizeof...(fltrs) > 0)
                intersectRec(fltrs...);
            if constexpr (sizeof...(excls) > 0)
                differenceRec(excls...);
        }

        [[nodiscard]] constexpr size_t size() const noexcept {
            return ents.size();
        }

        [[nodiscard]] constexpr const std::vector<Ent>::iterator begin() noexcept {
            return ents.begin();
        }

        [[nodiscard]] constexpr const std::vector<Ent>::iterator end() noexcept {
            return ents.end();
        }

        template <typename Func>
        void each(const Func& func) const {
            for ([[maybe_unused]] auto ent: ents) {
                try {
                    if constexpr (std::is_invocable<Func, Ent, Comp&, Comps&...>::value) {
                        std::apply(func, std::tuple_cat(std::make_tuple(ent), genTuple<std::tuple<>, decltype(pools)>(ent, std::tuple<>())));
                    }

                    else if constexpr (std::is_invocable<Func, Comp&, Comps&...>::value) {
                        std::apply(func, genTuple<std::tuple<>, decltype(pools)>(ent, std::tuple<>()));
                    }

                    else if constexpr (std::is_invocable<Func, Ent>::value) {
                        std::apply(func, std::make_tuple(ent));
                    }

                    else {
                        func();
                    }
                }

                catch (const std::exception& e) {
                    continue;
                }
            }
        }

    private:
        template <typename Tup, typename Pools, size_t Index = 0>
        [[nodiscard]] const decltype(auto) genTuple(Ent ent, Tup tup) const noexcept(false) {
            try {
                auto newTup = std::tuple_cat(tup, std::make_tuple(std::ref(std::get<Index>(pools).get(ent))));

                if constexpr (Index < std::tuple_size<Pools>::value - 1)
                    return genTuple<decltype(newTup), Pools, Index + 1>(ent, newTup);
                else
                    return newTup;
            }

            catch (const std::exception& e) {
                throw e;
            }
        }

        template <typename Pool>
        constexpr void genEnts(Pool& comp) {
            ents = comp.getEnts();
        }

        template <typename CompPool, typename... CompPools>
        constexpr void intersectRec(CompPool& comp, CompPools&... compPools) noexcept {
            intersectWith(comp.getEnts());
            if constexpr (sizeof...(CompPools) > 0)
                intersectRec(compPools...);
        }

        template <typename ExclPool, typename... ExclPools>
        constexpr void differenceRec(ExclPool& excl, ExclPools&... exclPools) noexcept {
            differenceWith(excl.getEnts());
            if constexpr (sizeof...(ExclPools) > 0)
                differenceRec(exclPools...);
        }

        constexpr void intersectWith(const std::vector<Ent>& other) noexcept {
            std::vector<Ent> newEnts;
            std::set_intersection(
                ents.begin(), ents.end(),
                other.begin(), other.end(),
                std::back_inserter(newEnts)
            );
            ents = newEnts;
        }

        constexpr void differenceWith(const std::vector<Ent>& other) noexcept {
            std::vector<Ent> newEnts;
            std::set_difference(
                ents.begin(), ents.end(),
                other.begin(), other.end(),
                std::back_inserter(newEnts)
            );
            ents = newEnts;
        }

    private:
        std::tuple<priv::CompPool<Comp>&, priv::CompPool<Comps>&...> pools;
        std::vector<Ent> ents;
    };
}

#endif // ZERENGINE_VIEW_HPP