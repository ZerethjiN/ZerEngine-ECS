/**
 * @file Systems.hpp
 * @author ZerethjiN
 * @brief The systems in-game, they all running each frame.
 * @version 0.1
 * @date 2022-02-22
 * 
 * @copyright Copyright (c) 2022 - ZerethjiN
 * 
 */
#ifndef ZERENGINE_SYSTEMS_HPP
#define ZERENGINE_SYSTEMS_HPP

#include <vector>
#include <functional>
#include "Registry.hpp"

namespace zre {
    struct World;

    namespace priv {
        class Sys {
        public:
            /**
             * @brief Add a new System that runs only once at startup.
             * 
             * @param func 
             */
            void addStartSys(const std::function<void(zre::World&)>& func) noexcept {
                startSystems.push_back(func);
            }

            /**
             * @brief Add a new System running each frame.
             * 
             * @tparam Args 
             * @param args 
             */
            template <typename... Args>
            constexpr void addSys(const Args&... args) noexcept {
                addSysRec(args...);
            }

            /**
             * @brief Run all Startup Systems.
             * 
             * @param world 
             */
            void start(zre::World& world) const noexcept {
                for (auto& func: startSystems) {
                    func(world);
                }
            }

            /**
             * @brief Run all Systems.
             * 
             * @param world 
             */
            void run(zre::World& world) const noexcept {
                for (auto& func: systems) {
                    func(world);
                }
            }

        private:
            /**
             * @brief Add new Systems Recursively.
             * 
             * @tparam Func 
             * @tparam Args 
             * @param func 
             * @param args 
             */
            template <typename Func, typename... Args>
            constexpr void addSysRec(const Func& func, const Args&... args) noexcept {
                systems.push_back(func);

                if constexpr (sizeof...(Args) > 0)
                    addSysRec(std::forward<Args>(args)...);
            }

        private:
            std::vector<std::function<void(zre::World&)>> startSystems;
            std::vector<std::function<void(zre::World&)>> systems;
        };
    }
}

#endif // ZERENGINE_SYSTEMS_HPP