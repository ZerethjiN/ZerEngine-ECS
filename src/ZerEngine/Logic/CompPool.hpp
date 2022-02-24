/**
 * @file CompPool.hpp
 * @author ZerethjiN
 * @brief packed all components of the same type
 * in a row in memory.
 * @version 0.1
 * @date 2022-02-22
 * 
 * @copyright Copyright (c) 2022 - ZerethjiN
 * 
 */
#ifndef ZERENGINE_COMP_POOL_HPP
#define ZERENGINE_COMP_POOL_HPP

#include <vector>
#include <unordered_map>
#include <algorithm>
#include "TypeUtilities.hpp"

namespace zre {
    namespace priv {
        class ICompPool {
        public:
            virtual ~ICompPool() noexcept = default;
            virtual void destroy([[maybe_unused]] Ent id) noexcept {};
        };

        template <typename T>
        class CompPool: public ICompPool {
        public:
            /**
             * @brief Add a new Component to this Pool,
             * with its Entity and Attributes.
             * 
             * @tparam Args
             * @param id The Entity of the New Component.
             * @param args Arguments needed to construct the New Component.
             */
            template <typename... Args>
            void add(const Ent id, Args&&... args) noexcept {
                packedComp.emplace_back(T{std::forward<Args>(args)...});
                entIndex.emplace(id, packedComp.size() - 1);
                indexEnt.emplace(packedComp.size() - 1, id);
                packedEnts.emplace_back(id);
                sort(packedEnts.begin(), packedEnts.end());
            }

            /**
             * @brief Delete a Component from this Pool by its Entity.
             * 
             * @param id The Entity of the Component to be Deleted.
             */
            void del(Ent id) noexcept {
                auto posRemoveInfo = entIndex[id];
                auto posLastInfo = static_cast<uint32_t>(packedComp.size() - 1);
                packedComp[posRemoveInfo] = packedComp[posLastInfo];

                auto entLastInfo = indexEnt[posLastInfo];
                entIndex[entLastInfo] = posRemoveInfo;
                indexEnt[posRemoveInfo] = entLastInfo;

                entIndex.erase(id);
                indexEnt.erase(posLastInfo);
                packedComp.pop_back();

                for (size_t i = 0; i < packedEnts.size(); i++) {
                    if (packedEnts[i] == id) {
                        packedEnts.erase(packedEnts.begin() + static_cast<long long int>(i));
                    }
                }
                sort(packedEnts.begin(), packedEnts.end());
            }

            /**
             * @brief Delete all components attached to an Entity.
             * 
             * @param id The Entity to be destroyed.
             */
            constexpr void destroy(Ent id) noexcept override {
                del(id);
            }

            /**
             * @brief Get a List of all the Entities in this Pool.
             * 
             * @return List of all Entities.
             */
            [[nodiscard]] constexpr const std::vector<Ent>& getEnts() const noexcept {
                return packedEnts;
            }

            /**
             * @brief Get a Component of this Pool by its Entity Id.
             * 
             * @param id The Entity Id.
             * @return The Component.
             */
            [[nodiscard]] constexpr T& get(Ent id) {
                return packedComp.at(entIndex.at(id));
            }

            /**
             * @brief Get a Component of this Pool by its Entity Id.
             * 
             * @param id The Entity Id.
             * @return The Component.
             */
            [[nodiscard]] constexpr const T& get(Ent id) const {
                return packedComp.at(entIndex.at(id));
            }

        private:
            std::vector<T> packedComp; // Packaged components.
            std::unordered_map<Ent, uint32_t> entIndex; // Relation between Entity and PackedComp Index.
            std::unordered_map<uint32_t, Ent> indexEnt; // Relation between PackedComp Index and Entity.
            std::vector<Ent> packedEnts; // List of all Entities connect to this Pool.
        };
    }
}

#endif // ZERENGINE_COMP_POOL_HPP