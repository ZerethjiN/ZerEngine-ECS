#ifndef ZERENGINE_LOGIC_SYSTEMS_SYSTEMS_HPP
#define ZERENGINE_LOGIC_SYSTEMS_SYSTEMS_HPP

#include "Threadpool.hpp"
#include "RenderThread.hpp"

namespace zre {
    namespace priv {
        struct Sys {
        public:
            /**
             * @brief Construct a new Sys object.
             * 
             * @param world 
             */
            inline Sys(World& world) noexcept:
                renderCopyFunc(nullptr),
                renderFunc(nullptr),
                pool(world, std::thread::hardware_concurrency() - 1),
                renderThread(world) {
            }

            /**
             * @brief Add a new System that runs only once at startup.
             * 
             * @param func 
             */
            constexpr void addStartSys(void(*const func)(World&)) noexcept {
                startSystems.emplace_back(func);
            }

            constexpr void addMainSys(void(*const func)(World&)) noexcept {
                mainSystems.emplace_back(nullptr, func);
            }

            constexpr void addMainCondSys(bool(*const cond)(World&), void(*const sys)(World&)) noexcept {
                mainSystems.emplace_back(cond, sys);
            }

            constexpr void addRenderCopy(void(*newRenderCopyFunc)(World&, LiteRegistry&)) noexcept {
                renderCopyFunc = newRenderCopyFunc;
            }

            constexpr void addRender(void(*newRenderFunc)(World&, LiteRegistry&)) noexcept {
                renderFunc = newRenderFunc;
            }

            /**
             * @brief Add a new System running each frame.
             * 
             * @tparam Args 
             * @param args 
             */
            template <typename... Args>
            constexpr void addSys(const Args&... args) noexcept {
                systems.emplace_back(nullptr, std::vector<void(*)(World&)>{args...});
            }

            template <typename... Args>
            constexpr void addCondSys(bool(*const cond)(World&), const Args&... args) noexcept {
                systems.emplace_back(cond, std::vector<void(*)(World&)>{args...});
            }

            /**
             * @brief Add a new System running each frame.
             * 
             * @tparam Args 
             * @param args 
             */
            template <typename... Args>
            constexpr void addLateSys(const Args&... args) noexcept {
                lateSystems.emplace_back(nullptr, std::vector<void(*)(World&)>{args...});
            }

            template <typename... Args>
            constexpr void addLateCondSys(bool(*const cond)(World&), const Args&... args) noexcept {
                lateSystems.emplace_back(cond, std::vector<void(*)(World&)>{args...});
            }

            /**
             * @brief Run all Startup Systems.
             * 
             * @param world 
             */
            constexpr void start(World& world) const noexcept {
                for (auto& func: startSystems) {
                    func(world);
                }
            }

            /**
             * @brief Run all Systems.
             * 
             * @param world 
             */
            constexpr void run(World& world) noexcept {
                if (renderCopyFunc != nullptr && renderFunc != nullptr) {
                    renderThread.copyData(world, renderCopyFunc);
                    renderThread.startRender(renderFunc);
                }

                for (const auto& mainFunc: mainSystems) {
                    if (mainFunc.first == nullptr || mainFunc.first(world)) {
                        mainFunc.second(world);
                    }
                }

                for (const auto& funcs: systems) {
                    if (funcs.first == nullptr || funcs.first(world)) {
                        for (auto& func: funcs.second) {
                            pool.addTask(func);
                        }
                        pool.wait();
                    }
                }

                if (renderCopyFunc != nullptr && renderFunc != nullptr) {
                    renderThread.wait();
                }

                for (const auto& lateFuncs: lateSystems) {
                    if (lateFuncs.first == nullptr || lateFuncs.first(world)) {
                        for (auto& func: lateFuncs.second) {
                            func(world);
                        }
                    }
                }
            }

        private:
            std::vector<void(*)(World&)> startSystems;
            std::vector<std::pair<bool(*)(World&), void(*)(World&)>> mainSystems;
            std::vector<std::pair<bool(*)(World&), std::vector<void(*)(World&)>>> systems;
            std::vector<std::pair<bool(*)(World&), std::vector<void(*)(World&)>>> lateSystems;
            void(*renderCopyFunc)(World&, LiteRegistry&);
            void(*renderFunc)(World&, LiteRegistry&);

            ThreadPool pool;
            RenderThread renderThread;
        };
    }
}

#endif /** ZERENGINE_LOGIC_SYSTEMS_SYSTEMS_HPP */
