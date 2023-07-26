#pragma once

#include "World.hpp"

class ZerEngine final {
public:
    template <typename T, typename... Args>
    [[nodiscard]] constexpr ZerEngine& addRes(const Args&... args) noexcept {
        world.res.emplace<T>(args...);
        return *this;
    }

    [[nodiscard]] constexpr ZerEngine& addStartSys(void(*const func)(World&)) noexcept {
        world.sys.addStartSys(func);
        return *this;
    }

    [[nodiscard]] constexpr ZerEngine& addMainSys(void(*const func)(World&)) noexcept {
        world.sys.addMainSys(func);
        return *this;
    }

    [[nodiscard]] constexpr ZerEngine& addMainCondSys(bool(*const cond)(World&), void(*const func)(World&)) noexcept {
        world.sys.addMainCondSys(cond, func);
        return *this;
    }

    template <typename... Args>
    [[nodiscard]] constexpr ZerEngine& addSys(const Args&... args) noexcept {
        world.sys.addSys(args...);
        return *this;
    }

    template <typename... Args>
    [[nodiscard]] constexpr ZerEngine& addCondSys(bool(*const cond)(World&), const Args&... args) noexcept {
        world.sys.addCondSys(cond, args...);
        return *this;
    }

    template <typename... Args>
    [[nodiscard]] constexpr ZerEngine& addLateSys(const Args&... args) noexcept {
        world.sys.addLateSys(args...);
        return *this;
    }

    template <typename... Args>
    [[nodiscard]] constexpr ZerEngine& addLateCondSys(bool(*const cond)(World&), const Args&... args) noexcept {
        world.sys.addLateCondSys(cond, args...);
        return *this;
    }

#ifdef ZERENGINE_USE_RENDER_THREAD
    [[nodiscard]] constexpr ZerEngine& addRenderCopy(void(*newRenderCopyFunc)(World&, LiteRegistry&)) noexcept {
        world.sys.addRenderCopy(newRenderCopyFunc);
        return *this;
    }

    [[nodiscard]] constexpr ZerEngine& addRender(void(*newRenderFunc)(World&, LiteRegistry&)) noexcept {
        world.sys.addRender(newRenderFunc);
        return *this;
    }
#endif

    constexpr void run() noexcept {
        world.isRunning = true;
        world.sys.start(world);
        while (world.isRunning) {
            world.upgrade();
            world.sys.run(world);
        }
        world.upgrade();
    }

private:
    World world;
};