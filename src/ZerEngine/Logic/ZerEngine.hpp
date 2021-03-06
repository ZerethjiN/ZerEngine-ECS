/**
 * @file ZerEngine.hpp
 * @author ZerethjiN
 * @brief An init application for the game.
 * @version 0.1
 * @date 2022-02-22
 * 
 * @copyright Copyright (c) 2022 - ZerethjiN
 * 
 */
#ifndef ZERENGINE_ZERENGINE_HPP
#define ZERENGINE_ZERENGINE_HPP

#include "World.hpp"

namespace zre {
    class ZerEngine {
    public:
        /**
         * @brief Run a plugin to add Entities or Systems on this application.
         * 
         * @param func 
         * @return ZerEngine& 
         */
        ZerEngine& addPlugin(const std::function<void(zre::ZerEngine&)>& func) noexcept {
            func(*this);
            return *this;
        }

        /**
         * @brief Add a new resource with its type.
         * 
         * @tparam T 
         * @param t 
         * @return
         */
        template <typename T, typename... Args>
        constexpr ZerEngine& addRes(Args&&... args) noexcept {
            world.res.emplace<T>(std::forward<Args>(args)...);
            return *this;
        }

        /**
         * @brief Add a new System that runs only once at startup.
         * 
         * @param func 
         * @return ZerEngine& 
         */
        ZerEngine& addStartSys(const std::function<void(zre::World&)>& func) noexcept {
            world.sys.addStartSys(func);
            return *this;
        }

        /**
         * @brief Add a new System running each frame.
         * 
         * @tparam Args 
         * @param args 
         * @return constexpr ZerEngine& 
         */
        template <typename... Args>
        constexpr ZerEngine& addSys(const Args&... args) noexcept {
            world.sys.addSys(args...);
            return *this;
        }

        template <typename... Args>
        constexpr ZerEngine& addCondSys(const std::function<bool(World&)>& func, const Args&... args) noexcept {
            world.sys.addCondSys(func, args...);
            return *this;
        }

        /**
         * @brief Run this application.
         * 
         * @return ZerEngine& 
         */
        ZerEngine& run() noexcept {
            world.isRunning = true;
            world.sys.start(world);
            while (world.isRunning) {
                world.sys.run(world);
            }
            return *this;
        }

    private:
        zre::World world;
    };
}

#endif // ZERENGINE_ZERENGINE_HPP