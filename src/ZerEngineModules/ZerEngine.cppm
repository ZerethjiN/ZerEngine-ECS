module;

#include <concepts>
#include <any>

export module ZerengineCore:ZerEngine;

import :World;

export class ZerEngine final {
public:
    template <typename T, typename... Args> requires (std::copy_constructible<T>)
    [[nodiscard]] ZerEngine& addRes(Args&&... args) noexcept {
        world.res.emplace(typeid(T).hash_code(), std::make_any<T>(std::forward<Args>(args)...));
        return *this;
    }

    template <typename T, typename... Args>
    [[nodiscard]] ZerEngine& addPlugin(void(*pluginFunc)(ZerEngine&)) noexcept {
        pluginFunc(*this);
        return *this;
    }

    [[nodiscard]] ZerEngine& addStartSys(void(*const func)(World&)) noexcept {
        world.sys.addStartSys(func);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(World&)> || std::same_as<Funcs, void(&)(World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addMainSys(Funcs&&... funcs) noexcept {
        world.sys.addMainSys(std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(World&)> || std::same_as<Funcs, void(&)(World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addMainCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        world.sys.addMainCondSys(cond, std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(World&)> || std::same_as<Funcs, void(&)(World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addThreadedSys(Funcs&&... funcs) noexcept {
        world.sys.addThreadedSys(std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(World&)> || std::same_as<Funcs, void(&)(World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addThreadedCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        world.sys.addThreadedCondSys(cond, std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(World&)> || std::same_as<Funcs, void(&)(World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addLateSys(Funcs&&... funcs) noexcept {
        world.sys.addLateSys(std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(World&)> || std::same_as<Funcs, void(&)(World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addLateCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        world.sys.addLateCondSys(cond, std::forward<Funcs>(funcs)...);
        return *this;
    }

    void run() noexcept {
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