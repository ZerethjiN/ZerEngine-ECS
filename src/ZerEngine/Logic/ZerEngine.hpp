/**
 * @file ZerEngine.hpp
 * @author ZerethjiN
 * @brief 
 * @version 0.2
 * @date 2022-07-18
 * 
 * @copyright Copyright (c) 2022 - ZerethjiN
 */
#ifndef ZERENGINE_ZERENGINE_HPP
#define ZERENGINE_ZERENGINE_HPP

#include "World.hpp"

namespace zre {
    class ZerEngine {
    public:
        /**
         * @brief Run a plugin to add Entities or Systems on this application.
         */
        [[nodiscard]] constexpr ZerEngine& addPlugin(void(*const func)(zre::ZerEngine&)) noexcept {
            func(*this);
            return *this;
        }

        /**
         * @brief Add a new resource with its type.
         */
        template <typename T, typename... Args>
        [[nodiscard]] constexpr ZerEngine& addRes(Args&&... args) noexcept {
            world.getRes().emplace<T>(std::forward<Args>(args)...);
            return *this;
        }

        /**
         * @brief Add a new System that runs only once at startup.
         */
        [[nodiscard]] constexpr ZerEngine& addStartSys(void(*const func)(World&)) noexcept {
            world.getSys().addStartSys(func);
            return *this;
        }

        [[nodiscard]] constexpr ZerEngine& addMainSys(void(*const func)(World&)) noexcept {
            world.getSys().addMainSys(func);
            return *this;
        }

        [[nodiscard]] constexpr ZerEngine& addCondSys(bool(*const cond)(World&), void(*const func)(World&)) noexcept {
            world.getSys().addMainCondSys(cond, func);
            return *this;
        }

        [[nodiscard]] constexpr ZerEngine& addLateSys(void(*const func)(World&)) noexcept {
            world.getSys().addLateSys(func);
            return *this;
        }

        [[nodiscard]] constexpr ZerEngine& addLateCondSys(bool(*const cond)(World&), void(*const func)(World&)) noexcept {
            world.getSys().addLateCondSys(cond, func);
            return *this;
        }

        /**
         * @brief Add a new System running each frame.
         */
        template <typename... Args>
        [[nodiscard]] constexpr ZerEngine& addSys(const Args&... args) noexcept {
            world.getSys().addSys(args...);
            return *this;
        }

        /**
         * @brief Add a new System running each frame only if the condition is valid.
         */
        template <typename... Args>
        [[nodiscard]] constexpr ZerEngine& addCondSys(bool(*const cond)(World&), const Args&... args) noexcept {
            world.getSys().addCondSys(cond, args...);
            return *this;
        }

        /**
         * @brief Add a new System running each frame.
         */
        template <typename... Args>
        [[nodiscard]] constexpr ZerEngine& addLateSys(const Args&... args) noexcept {
            world.getSys().addLateSys(args...);
            return *this;
        }

        /**
         * @brief Add a new System running each frame only if the condition is valid.
         */
        template <typename... Args>
        [[nodiscard]] constexpr ZerEngine& addLateCondSys(bool(*const cond)(World&), const Args&... args) noexcept {
            world.getSys().addLateCondSys(cond, args...);
            return *this;
        }

        /**
         * @brief Add a new System to Copy the Registry Data for the Rendering Thread.
         */
        [[nodiscard]] constexpr ZerEngine& addRenderCopy(void(*newRenderCopyFunc)(World&, priv::LiteRegistry&)) noexcept {
            world.getSys().addRenderCopy(newRenderCopyFunc);
            return *this;
        }

        /**
         * @brief Add a new Rendering System.
         */
        [[nodiscard]] constexpr ZerEngine& addRender(void(*newRenderFunc)(World&, priv::LiteRegistry&)) noexcept {
            world.getSys().addRender(newRenderFunc);
            return *this;
        }

        /**
         * @brief Run this application.
         */
        constexpr void run() noexcept {
            world.setIsRunning(true);
            world.getSys().start(world);
            while (world.getIsRunning()) {
                world.upgrade();
                world.getSys().run(world);
            }
        }

    private:
        World world;
    };
}

#endif /** ZERENGINE_ZERENGINE_HPP */
