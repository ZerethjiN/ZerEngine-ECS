/**
 * @file World.hpp
 * @author ZerethjiN
 * @brief An exchangeable object that contains all
 * resources for the game.
 * @version 0.1
 * @date 2022-02-22
 * 
 * @copyright Copyright (c) 2022 - ZerethjiN
 * 
 */
#ifndef ZERENGINE_WORLD_HPP
#define ZERENGINE_WORLD_HPP

#include "Registry.hpp"
#include "TypeMap.hpp"
#include "Systems.hpp"

namespace zre {
    class World {
    public:
        World():
            res(),
            reg(),
            sys(*this),
            isRunning(false) {
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

        template <typename T>
        [[nodiscard]] constexpr bool contains(const Ent id) const noexcept {
            return reg.contains<T>(id);
        }

        /**
         * @brief Get a component by its entity and type.
         * 
         * @tparam T 
         * @param ent 
         * @return constexpr T& 
         */
        template <typename T>
        [[nodiscard]] constexpr T& get(Ent ent) noexcept {
            return reg.get<T>(ent);
        }

        /**
         * @brief Get a constant component by its entity and type.
         * 
         * @tparam T 
         * @param ent 
         * @return constexpr const T& 
         */
        template <typename T>
        [[nodiscard]] constexpr const T& get(Ent ent) const noexcept {
            return reg.get<T>(ent);
        }

        /**
         * @brief Get a new Query per necessary Component, Filter and Exclusion Type.
         * 
         * @tparam Comp 
         * @tparam Comps 
         * @tparam Filters 
         * @tparam Excludes 
         * @param filter 
         * @param exclude 
         * @return constexpr const Query<Comp, priv::comp_t<Comps...>, priv::With<Filters...>, priv::Without<Excludes...>> 
         */
        template <typename Comp, typename... Comps, typename... Filters, typename... Excludes, typename... Optionnals>
        [[nodiscard]] constexpr const Query<Comp, priv::comp_t<Comps...>, priv::With<Filters...>, priv::Without<Excludes...>, priv::OrWith<Optionnals...>> query(priv::With<Filters...> filter = {}, priv::Without<Excludes...> exclude = {}, priv::OrWith<Optionnals...> option = {}) noexcept {
            return reg.query<Comp>(priv::comp_t<Comps...>(), filter, exclude, option);
        }

        /**
         * @brief Get a new Query per necessary Component, Filter and Exclusion Type.
         * 
         * @tparam Comp 
         * @tparam Comps 
         * @tparam Filters 
         * @tparam Excludes 
         * @param exclude 
         * @param filter 
         * @return constexpr const Query<Comp, priv::comp_t<Comps...>, priv::With<Filters...>, priv::Without<Excludes...>> 
         */
        template <typename Comp, typename... Comps, typename... Filters, typename... Excludes, typename... Optionnals>
        [[nodiscard]] constexpr const Query<Comp, priv::comp_t<Comps...>, priv::With<Filters...>, priv::Without<Excludes...>, priv::OrWith<Optionnals...>> query(priv::Without<Excludes...> exclude, priv::With<Filters...> filter = {}, priv::OrWith<Optionnals...> option = {}) noexcept {
            return reg.query<Comp>(priv::comp_t<Comps...>(), filter, exclude, option);
        }

        /**
         * @brief Get a new Query per necessary Component, Filter and Exclusion Type.
         * 
         * @tparam Comp 
         * @tparam Comps 
         * @tparam Filters 
         * @tparam Excludes 
         * @param exclude 
         * @param filter 
         * @return constexpr const Query<Comp, priv::comp_t<Comps...>, priv::With<Filters...>, priv::Without<Excludes...>> 
         */
        template <typename Comp, typename... Comps, typename... Filters, typename... Excludes, typename... Optionnals>
        [[nodiscard]] constexpr const Query<Comp, priv::comp_t<Comps...>, priv::With<Filters...>, priv::Without<Excludes...>, priv::OrWith<Optionnals...>> query(priv::OrWith<Optionnals...> option, priv::Without<Excludes...> exclude = {}, priv::With<Filters...> filter = {}) noexcept {
            return reg.query<Comp>(priv::comp_t<Comps...>(), filter, exclude, option);
        }

        /**
         * @brief Get a new Query per necessary Component, Filter and Exclusion Type.
         * 
         * @tparam Comp 
         * @tparam Comps 
         * @tparam Filters 
         * @tparam Excludes 
         * @param exclude 
         * @param filter 
         * @return constexpr const Query<Comp, priv::comp_t<Comps...>, priv::With<Filters...>, priv::Without<Excludes...>> 
         */
        template <typename Comp, typename... Comps, typename... Filters, typename... Excludes, typename... Optionnals>
        [[nodiscard]] constexpr const Query<Comp, priv::comp_t<Comps...>, priv::With<Filters...>, priv::Without<Excludes...>, priv::OrWith<Optionnals...>> query(priv::OrWith<Optionnals...> option, priv::With<Filters...> filter, priv::Without<Excludes...> exclude = {}) noexcept {
            return reg.query<Comp>(priv::comp_t<Comps...>(), filter, exclude, option);
        }

        /**
         * @brief Emplace a new Component to an Entity bases on its Component Type and Entity Id.
         * 
         * @tparam T 
         * @tparam Args 
         * @param id 
         * @param args 
         */
        template <typename T, typename... Args>
        constexpr void emplace(Ent id, Args&&... args) noexcept {
            reg.emplace<T>(id, std::forward<Args>(args)...);
        }

        /**
         * @brief Add new Components to an Entity bases.
         * 
         * @tparam T 
         * @tparam Args 
         * @param id 
         * @param args 
         */
        template <typename T, typename... Args>
        constexpr void add(Ent id, Args&&... args) noexcept {
            reg.add<T>(id, std::forward<Args>(args)...);
        }

        /**
         * @brief Delete a Component from an Entity bases on its Component Type and Entity Id.
         * 
         * @tparam T 
         * @param ent 
         */
        template <typename T>
        constexpr void del(Ent ent) noexcept {
            reg.del<T>(ent);
        }

        /**
         * @brief Delete one component and replace it with another based on their Component Type and Entity Id.
         * 
         * @tparam T1 
         * @tparam T2 
         * @tparam Args 
         * @param ent 
         * @param args 
         */
        template <typename T1, typename T2, typename... Args>
        constexpr void rep(const Ent ent, Args&&... args) noexcept {
            reg.del<T1>(ent);
            reg.add<T2>(ent, std::forward<Args>(args)...);
        }

        /**
         * @brief Create a new Entity with its components.
         * 
         * @tparam Args 
         * @param args 
         * @return constexpr Ent 
         */
        template <typename... Args>
        constexpr Ent newEnt(Args&&... args) noexcept {
            const auto ent = reg.newEnt();

            if constexpr (sizeof...(Args) > 0)
                instanciateRec(ent, std::forward<Args>(args)...);

            return ent;
        }

        /**
         * @brief Destroys an Entity.
         * 
         * @param ent 
         */
        void destroy(Ent ent) noexcept {
            reg.destroy(ent);
        }

        void destroyAll() noexcept {
            reg.destroyAll();
        }

        /**
         * @brief Stop the Game loop.
         * 
         * @param val 
         */
        constexpr void stopRun(bool val = false) noexcept {
            isRunning = val;
        }

    private:
        template <typename Arg, typename... Args>
        constexpr void instanciateRec(Ent id, Arg&& arg, Args&&... args) noexcept {
            reg.add<Arg>(id, std::forward<Arg>(arg));

            if constexpr (sizeof...(Args) > 0)
                instanciateRec(id, std::forward<Args>(args)...);
        }

    public:
        priv::Res res;
        priv::Reg reg;
        priv::Sys sys;
        bool isRunning;
    };
}

#endif // ZERENGINE_WORLD_HPP