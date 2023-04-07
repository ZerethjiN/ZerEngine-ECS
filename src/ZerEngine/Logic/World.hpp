/**
 * @file World.hpp
 * @author ZerethjiN
 * @brief An exchangeable object that contains all
 * resources for the game.
 * @version 0.2
 * @date 2022-07-18
 * 
 * @copyright Copyright (c) 2022 - ZerethjiN
 */
#ifndef ZERENGINE_WORLD_HPP
#define ZERENGINE_WORLD_HPP

#include "Registry/Registry.hpp"
#include "Systems/Systems.hpp"
#include "../Utils/TypeMap.hpp"
#include "LateUpgrade/LateUpgrade.hpp"

namespace zre {
    class World {
    public:
        inline World() noexcept:
            sys(*this) {
        }

        template <typename T>
        [[nodiscard]] constexpr bool contains(const Ent id) const noexcept {
            return reg.contains<T>(id);
        }

        template <typename T>
        [[nodiscard]] constexpr T& get(Ent ent) noexcept {
            return reg.get<T>(ent);
        }

        template <typename T>
        [[nodiscard]] constexpr const T& get(Ent ent) const noexcept {
            return reg.get<T>(ent);
        }

        /**
         * @brief Get a resource by its type.
         * 
         * @tparam T 
         * @return constexpr T& 
         */
        template <typename T>
        [[nodiscard]] constexpr T& getRes() noexcept {
            return res.get<T>();
        }

        /**
         * @brief Get a constant resource by its type.
         * 
         * @tparam T 
         * @return constexpr T& 
         */
        template <typename T>
        [[nodiscard]] constexpr const T& getRes() const noexcept {
            return res.get<T>();
        }

        template <typename... Comps, typename... Filters, typename... Excludes>
        [[nodiscard]] constexpr const zre::Query<Comps...> query(zre::With<Filters...> filters = {}, zre::Without<Excludes...> excludes = {}) {
            return reg.query<Comps...>(filters, excludes);
        }

        template <typename... Comps, typename... Filters, typename... Excludes>
        [[nodiscard]] constexpr const zre::Query<Comps...> query(zre::Without<Excludes...> excludes, zre::With<Filters...> filters = {}) {
            return reg.query<Comps...>(filters, excludes);
        }

        template <typename... Comps>
        constexpr void newEnt(const Comps&... comps) noexcept {
            lateUpgrade.newEnt(std::forward<const Comps>(comps)...);
        }

        template <typename... Comps>
        constexpr Ent directNewEnt(const Comps&... comps) noexcept {
            return reg.directNewEnt(comps...);
        }

        template <typename Arg>
        constexpr void add(zre::Ent id, const Arg& arg) noexcept {
            lateUpgrade.add(id, std::forward<const Arg>(arg));
        }

        template <typename T, typename... Args>
        constexpr void emplace(zre::Ent id, const Args&... args) noexcept {
            lateUpgrade.add(id, std::forward<const T>(T(args...)));
        }

        template <typename T>
        constexpr void del(zre::Ent id) noexcept {
            lateUpgrade.del<T>(id);
        }

        template <typename Old, typename New>
        constexpr void replace(zre::Ent id, const New& newComp) noexcept {
            lateUpgrade.add(id, newComp);
            lateUpgrade.del<Old>(id);
        }

        constexpr void destroy(zre::Ent ent) noexcept {
            lateUpgrade.destroy(ent);
        }

        constexpr void stopRun(bool val = true) noexcept {
            isRunning = !val;
        }

    public:
        [[nodiscard]] constexpr zre::priv::Sys& getSys() noexcept {
            return sys;
        }

        [[nodiscard]] constexpr zre::priv::Res& getRes() noexcept {
            return res;
        }

        [[nodiscard]] constexpr bool getIsRunning() const noexcept {
            return isRunning;
        }

        constexpr void setIsRunning(bool state) noexcept {
            isRunning = state;
        }

        void upgrade() noexcept {
            lateUpgrade.upgrade(reg);
        }

    private:
        zre::priv::Registry reg;
        zre::priv::Res res;
        zre::priv::Sys sys;
        zre::priv::LateUpgrade lateUpgrade;
        bool isRunning;
    };
}

#endif /** ZERENGINE_WORLD_HPP*/