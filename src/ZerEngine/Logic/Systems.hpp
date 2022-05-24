/**
 * @file Systems.hpp
 * @author ZerethjiN
 * @brief The systems in-game, they all running each frame.
 * @version 0.1
 * @date 2022-02-22
 * 
 * @copyright Copyright (c) 2022 - ZerethjiN
 */
#ifndef ZERENGINE_SYSTEMS_HPP
#define ZERENGINE_SYSTEMS_HPP

#include "Threadpool.hpp"
#include "Registry.hpp"

namespace zre {
    namespace priv {
        class Sys {
        public:
            /**
             * @brief Construct a new Sys object.
             * 
             * @param world 
             */
            Sys(World& world):
                pool(world) {
            }

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
                systems.push_back(std::pair<std::function<bool(zre::World&)>, std::vector<std::function<void(zre::World&)>>>(nullptr, {args...}));
            }

            template <typename... Args>
            constexpr void addCondSys(const std::function<bool(zre::World&)>& func, const Args&... args) noexcept {
                systems.push_back(std::pair<std::function<bool(zre::World&)>, std::vector<std::function<void(zre::World&)>>>(func, {args...}));
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
            void run(zre::World& world) noexcept {
                for (auto& funcs: systems) {
                    if (funcs.first == nullptr || funcs.first(world)) {
                        for (auto& func: funcs.second) {
                            pool.addTask(func);
                        }

                        pool.wait();
                    }
                }
            }

        private:
            std::vector<std::function<void(zre::World&)>> startSystems;
            std::vector<std::pair<std::function<bool(zre::World&)>, std::vector<std::function<void(zre::World&)>>>> systems;

            ThreadPool pool;
        };
    }
}

#endif // ZERENGINE_SYSTEMS_HPP