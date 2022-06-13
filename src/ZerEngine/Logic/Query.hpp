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
#include <iterator>
#include "TypeUtilities.hpp"
#include "CompPool.hpp"

namespace zre {
    template <typename, typename, typename, typename, typename>
    class Query;

    template <typename Comp, typename... Comps, typename... Filters, typename... Excludes, typename... Optionnals>
    class Query<Comp, priv::comp_t<Comps...>, priv::With<Filters...>, priv::Without<Excludes...>, priv::OrWith<Optionnals...>> {
    public:
        constexpr Query(priv::CompPool<Comp>& comp, priv::CompPool<Comps>&... comps, priv::CompPool<Filters>&... fltrs, priv::CompPool<Excludes>&... excls, priv::CompPool<Optionnals>&... options) noexcept:
            pools(comp, comps...) {
            genEnts(comp);
            if constexpr (sizeof...(comps) > 0)
                intersectRec(comps...);
            if constexpr (sizeof...(fltrs) > 0)
                intersectRec(fltrs...);
            if constexpr (sizeof...(options) > 0)
                intersectWith(unionRec(std::vector<Ent>(), options...));
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
        constexpr void each(const Func& func) const noexcept {
            for ([[maybe_unused]] const auto ent: ents) {
                if constexpr (std::is_invocable<Func, Ent, Comp&, Comps&...>::value) {
                    std::apply(func, genTuple<true, zre::priv::CompPool<Comp>&, zre::priv::CompPool<Comps>&...>(ent));
                }

                else if constexpr (std::is_invocable<Func, Comp&, Comps&...>::value) {
                    std::apply(func, genTuple<false, zre::priv::CompPool<Comp>&, zre::priv::CompPool<Comps>&...>(ent));
                }

                else if constexpr (std::is_invocable<Func, Ent>::value) {
                    std::apply(func, std::make_tuple(ent));
                }

                else {
                    func();
                }
            }
        }

    private:
        template <bool WithEnt, typename... Args>
        [[nodiscard]] constexpr decltype(auto) genTuple(const Ent ent) const noexcept {
            if constexpr (WithEnt)
                return std::make_tuple(ent, std::ref(std::get<Args>(pools).get(ent))...);
            else
                return std::make_tuple(std::ref(std::get<Args>(pools).get(ent))...);
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

        template <typename OrPool, typename... OrPools>
        const std::vector<Ent> unionRec(const std::vector<Ent> oldEnts, OrPool& orPool, OrPools&... orPools) noexcept {
            auto newEnts = unionWith(oldEnts, orPool.getEnts());
            if constexpr (sizeof...(OrPools) > 0)
                return unionRec(newEnts, orPools...);
            else
                return newEnts;
        }

        template <typename ExclPool, typename... ExclPools>
        constexpr void differenceRec(ExclPool& excl, ExclPools&... exclPools) noexcept {
            differenceWith(excl.getEnts());
            if constexpr (sizeof...(ExclPools) > 0)
                differenceRec(exclPools...);
        }

        void intersectWith(const std::vector<Ent>& other) noexcept {
            std::vector<Ent> tmpEnts;
            std::set_intersection(
                ents.begin(), ents.end(),
                other.begin(), other.end(),
                std::back_inserter(tmpEnts)
            );
            ents.swap(tmpEnts);
        }

        const std::vector<Ent> unionWith(const std::vector<Ent>& entA, const std::vector<Ent>& entB) noexcept {
            std::vector<Ent> tmpEnts;
            std::set_union(
                entA.begin(), entA.end(),
                entB.begin(), entB.end(),
                std::back_inserter(tmpEnts)
            );
            return tmpEnts;
        }

        void differenceWith(const std::vector<Ent>& other) noexcept {
            std::vector<Ent> tmpEnts;
            std::set_difference(
                ents.begin(), ents.end(),
                other.begin(), other.end(),
                std::back_inserter(tmpEnts)
            );
            ents.swap(tmpEnts);
        }

    private:
        const std::tuple<priv::CompPool<Comp>&, priv::CompPool<Comps>&...> pools;
        std::vector<Ent> ents;
    };
}

#endif // ZERENGINE_VIEW_HPP