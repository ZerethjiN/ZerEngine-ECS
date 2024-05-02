module;

#include <thread>
#include <vector>

export module ZerengineCore:Sys;

import :ThreadPool;

class Sys final {
friend class World;
friend class ZerEngine;
private:
    Sys(World& world) noexcept:
        threadpool(world, std::thread::hardware_concurrency() - 1)
    {
        srand(time(NULL));
    }

private:
    constexpr void addStartSys(void(*const func)(World&)) noexcept {
        startSystems.emplace_back(func);
    }

    template <typename... Funcs>
    constexpr void addMainSys(Funcs&&... funcs) noexcept {
        mainSystems.emplace_back(nullptr, std::initializer_list<void(*)(World&)>{std::forward<Funcs>(funcs)...});
    }

    template <typename... Funcs>
    constexpr void addMainCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        mainSystems.emplace_back(cond, std::initializer_list<void(*)(World&)>{std::forward<Funcs>(funcs)...});
    }

    template <typename... Funcs>
    constexpr void addThreadedSys(Funcs&&... funcs) noexcept {
        threadedSystems.emplace_back(nullptr, std::initializer_list<void(*)(World&)>{std::forward<Funcs>(funcs)...});
    }

    template <typename... Funcs>
    constexpr void addThreadedCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        threadedSystems.emplace_back(cond, std::initializer_list<void(*)(World&)>{std::forward<Funcs>(funcs)...});
    }

    template <typename... Funcs>
    constexpr void addLateSys(Funcs&&... funcs) noexcept {
        lateSystems.emplace_back(nullptr, std::initializer_list<void(*)(World&)>{std::forward<Funcs>(funcs)...});
    }

    template <typename... Funcs>
    constexpr void addLateCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        lateSystems.emplace_back(cond, std::initializer_list<void(*)(World&)>{std::forward<Funcs>(funcs)...});
    }

    void start(World& world) const noexcept {
        for (const auto& func: startSystems) {
            func(world);
        }
    }

    void run(World& world) noexcept {
        for (const auto& mainFunc: mainSystems) {
            if (mainFunc.first == nullptr || mainFunc.first(world)) {
                for (const auto& mainRow: mainFunc.second) {
                    mainRow(world);
                }
            }
        }

        for (const auto& funcs: threadedSystems) {
            if (funcs.first == nullptr || funcs.first(world)) {
    #ifdef ZER_NO_MULTITHREADING
                for (auto& func: funcs.second) {
                    func(world);
                }
    #else
                threadpool.addTasks(funcs.second);
    #endif
            }
        }

        threadpool.run();

        threadpool.wait();

        for (const auto& lateFunc: lateSystems) {
            if (lateFunc.first == nullptr || lateFunc.first(world)) {
                for (const auto& lateRow: lateFunc.second) {
                    lateRow(world);
                }
            }
        }
    }

private:
    std::vector<void(*)(World&)> startSystems;
    std::vector<std::pair<bool(*)(World&), std::vector<void(*)(World&)>>> mainSystems;
    std::vector<std::pair<bool(*)(World&), std::vector<void(*)(World&)>>> threadedSystems;
    std::vector<std::pair<bool(*)(World&), std::vector<void(*)(World&)>>> lateSystems;
    ThreadPool threadpool;
};