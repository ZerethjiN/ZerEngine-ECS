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
#include "Registry/LiteRegistry.hpp"

namespace zre {
    class World {
    public:
        inline World() noexcept:
            sys(*this),
            reg(sys),
            lateUpgrade(reg) {
        }

        template <typename T>
        [[nodiscard]] constexpr bool contains(const Ent ent) const noexcept {
            return reg.contains<T>(ent);
        }

        template <typename T>
        [[nodiscard]] constexpr T& get(const Ent ent) noexcept {
            return reg.get<T>(ent);
        }

        template <typename T>
        [[nodiscard]] constexpr const T& get(const Ent ent) const noexcept {
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
        [[nodiscard]] constexpr const zre::Query<Comps...> query(const zre::With<Filters...> filters = {}, const zre::Without<Excludes...> excludes = {}) {
            return reg.query<Comps...>(filters, excludes);
        }

        template <typename... Comps, typename... Filters, typename... Excludes>
        [[nodiscard]] constexpr const zre::Query<Comps...> query(const zre::Without<Excludes...> excludes, const zre::With<Filters...> filters = {}) {
            return reg.query<Comps...>(filters, excludes);
        }

        template <typename... Comps>
        constexpr Ent newEnt(Comps&&... comps) noexcept {
            return lateUpgrade.newEnt(std::forward<Comps>(comps)...);
        }

        constexpr Ent getEntToken() noexcept {
            return reg.getEntToken();
        }

        template <typename... Comps>
        constexpr Ent directNewEnt(Comps&&... comps) noexcept {
            return reg.directNewEnt(std::forward<Comps>(comps)...);
        }

        template <typename Arg>
        constexpr void add(const zre::Ent ent, Arg&& arg) noexcept {
            lateUpgrade.add(ent, std::forward<Arg>(arg));
        }

        template <typename T, typename... Args>
        constexpr void emplace(const zre::Ent ent, Args&&... args) noexcept {
            lateUpgrade.emplace<T>(ent, std::forward<Args>(args)...);
        }

        template <typename T>
        constexpr void del(const zre::Ent ent) noexcept {
            lateUpgrade.del<T>(ent);
        }

        template <typename Old, typename New>
        constexpr void replace(const zre::Ent ent, New&& newComp) noexcept {
            lateUpgrade.add(ent, std::forward<New>(newComp));
            lateUpgrade.del<Old>(ent);
        }

        constexpr void destroy(const zre::Ent ent) noexcept {
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

        [[nodiscard]] constexpr zre::priv::Registry& getReg() noexcept {
            return reg;
        }

        [[nodiscard]] constexpr bool getIsRunning() const noexcept {
            return isRunning;
        }

        constexpr void setIsRunning(bool state) noexcept {
            isRunning = state;
        }

        constexpr void upgrade() noexcept {
            lateUpgrade.upgrade();
        }

    private:
        zre::priv::Res res;
        zre::priv::Sys sys;
        zre::priv::Registry reg;
        zre::priv::LateUpgrade lateUpgrade;
        bool isRunning;
    };
}

#endif /** ZERENGINE_WORLD_HPP*/