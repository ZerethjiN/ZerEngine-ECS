/**
 * @file Registry.hpp
 * @author ZerethjiN
 * @brief Managed memory with component pool
 * and infinite entity token.
 * @version 0.1
 * @date 2022-02-22
 * 
 * @copyright Copyright (c) 2022 - ZerethjiN
 * 
 */
#ifndef ZERENGINE_REGISTRY_HPP
#define ZERENGINE_REGISTRY_HPP

#include <unordered_map>
#include <vector>
#include <queue>
#include <memory>
#include "Query.hpp"
#include "TypeUtilities.hpp"
#include "CompPool.hpp"

namespace zre {
    namespace priv {
        class Reg {
        public:
            /**
             * @brief Create a New Entity in this Registry.
             * 
             * @return The Id of the New Entity.
             */
            [[nodiscard]] Ent newEnt() noexcept {
                Ent ent;

                if (entTokens.empty()) {
                    ent = NB_ENTITY;
                    NB_ENTITY++;
                } else {
                    ent = entTokens.front();
                    entTokens.pop();
                }

                entComps.emplace(ent, std::vector<Type>());

                return ent;
            }

            /**
             * @brief Destroys All Components attached to an Entity and Releases its Id.
             * 
             * @param id The Entity to be Destroyed.
             */
            void destroy(Ent id) noexcept {
                auto& comps = entComps.at(id);

                for (size_t i = 0; i < comps.size(); i++) {
                    compPools.at(comps.at(i))->destroy(id);
                }

                entComps.erase(id);
                entTokens.push(id);
            }

            /**
             * @brief Add a new Component to an Entity bases on its Component Type and Entity Id.
             * 
             * @tparam T 
             * @tparam Args 
             * @param id Entity Id.
             * @param args Arguments to generate the new Component.
             */
            template <typename T, typename... Args>
            constexpr void add(const Ent id, Args&&... args) {
                entComps.at(id).push_back(typeid(T).hash_code());
                assure<T>().add(id, std::forward<Args>(args)...);
            }

            /**
             * @brief Delete a Component from an Entity bases on its Component Type and Entity Id.
             * 
             * @tparam T 
             * @param id 
             */
            template <typename T>
            constexpr void del(const Ent id) noexcept {
                assure<T>().del(id);
            }

            template <typename T>
            [[nodiscard]] constexpr bool contains(const Ent id) const noexcept {
                const Type type = typeid(T).hash_code();

                for (auto& entType: entComps.at(id)) {
                    if (entType == type) {
                        return true;
                    }
                }

                return false;
            }

            /**
             * @brief Get a component on an Entity bases on desired Component Type and Entity Id.
             * 
             * @tparam T 
             * @param id 
             * @return A Reference to the desired Component.
             */
            template <typename T>
            [[nodiscard]] constexpr T& get(Ent id) noexcept {
                return assure<T>().get(id);
            }

            /**
             * @brief Get a component on an Entity bases on desired Component Type and Entity Id.
             * 
             * @tparam T 
             * @param id 
             * @return A Reference to the desired Component.
             */
            template <typename T>
            [[nodiscard]] constexpr const T& get(Ent id) const noexcept {
                return assure<T>().get(id);
            }

            /**
             * @brief Generate a new Query per necessary Component, Filter and Exclusion Type.
             * 
             * @tparam Comp 
             * @tparam Comps 
             * @tparam Filters 
             * @tparam Excludes 
             * @return New Query generated.
             */
            template <typename Comp, typename... Comps, typename... Filters, typename... Excludes, typename... Optionnals>
            [[nodiscard]] constexpr const Query<Comp, comp_t<Comps...>, With<Filters...>, Without<Excludes...>, OrWith<Optionnals...>> query(comp_t<Comps...> = {}, With<Filters...> = {}, Without<Excludes...> = {}, OrWith<Optionnals...> = {}) noexcept {
                return {
                    assure<Comp>(),
                    assure<Comps>()...,
                    assure<Filters>()...,
                    assure<Excludes>()...,
                    assure<Optionnals>()...
                };
            }

        private:
            /**
             * @brief Get the right Pool of Components by Type.
             * 
             * @tparam T 
             * @return A Pool of Components.
             */
            template <typename T>
            [[nodiscard]] constexpr priv::CompPool<T>& assure() noexcept {
                regPool<T>();
                return *std::static_pointer_cast<priv::CompPool<T>>(compPools.at(typeid(T).hash_code()));
            }

            /**
             * @brief Check if a component pool exists or create it.
             * 
             * @tparam T 
             */
            template <typename T>
            constexpr void regPool() noexcept {
                const Type type = typeid(T).hash_code();

                if (!compPools.count(type)) {
                    compPools.emplace(type, std::make_shared<priv::CompPool<T>>());
                }
            }

        private:
            std::unordered_map<Type, std::shared_ptr<ICompPool>> compPools;
            std::unordered_map<Ent, std::vector<Type>> entComps;
            std::queue<Ent> entTokens;

            static inline Ent NB_ENTITY = 0;
        };
    }
}

#endif // ZERENGINE_REGISTRY_HPP