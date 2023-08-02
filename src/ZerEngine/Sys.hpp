#pragma once

#include <vector>
#include <thread>
#include "ThreadPool.hpp"
#ifdef ZERENGINE_USE_RENDER_THREAD
    #include "LiteRegistry.hpp"
    #include "RenderThread.hpp"
#endif

class Sys final {
public:
    inline Sys(World& world) noexcept:
#ifdef ZERENGINE_USE_RENDER_THREAD
        threadpool(world, std::thread::hardware_concurrency() - 2),
        renderCopyFunc(nullptr),
        renderFunc(nullptr),
        renderThread(world)
#else
        threadpool(world, std::thread::hardware_concurrency() - 1)
#endif
    {
    }

    constexpr void addStartSys(void(*const func)(World&)) noexcept {
        startSystems.emplace_back(func);
    }

    constexpr void addMainSys(void(*const func)(World&)) noexcept {
        mainSystems.emplace_back(nullptr, func);
    }

    constexpr void addMainCondSys(bool(*const cond)(World&), void(*const sys)(World&)) noexcept {
        mainSystems.emplace_back(cond, sys);
    }

#ifdef ZERENGINE_USE_RENDER_THREAD
    constexpr void addRenderCopy(void(*newRenderCopyFunc)(World&, LiteRegistry&)) noexcept {
        renderCopyFunc = newRenderCopyFunc;
    }

    constexpr void addRender(void(*newRenderFunc)(World&, LiteRegistry&)) noexcept {
        renderFunc = newRenderFunc;
    }
#endif

    template <typename... Args>
    constexpr void addSys(const Args&... args) noexcept {
        systems.emplace_back(
            std::piecewise_construct,
            std::forward_as_tuple(nullptr),
            std::forward_as_tuple<std::vector<void(*)(World&)>>({args...})
        );
    }

    template <typename... Args>
    constexpr void addCondSys(bool(*const cond)(World&), const Args&... args) noexcept {
        systems.emplace_back(
            std::piecewise_construct,
            std::forward_as_tuple(cond),
            std::forward_as_tuple<std::vector<void(*)(World&)>>({args...})
        );
    }

    template <typename... Args>
    constexpr void addLateSys(const Args&... args) noexcept {
        lateSystems.emplace_back(
            std::piecewise_construct,
            std::forward_as_tuple(nullptr),
            std::forward_as_tuple<std::vector<void(*)(World&)>>({args...})
        );
    }

    template <typename... Args>
    constexpr void addLateCondSys(bool(*const cond)(World&), const Args&... args) noexcept {
        lateSystems.emplace_back(
            std::piecewise_construct,
            std::forward_as_tuple(cond),
            std::forward_as_tuple<std::vector<void(*)(World&)>>({args...})
        );
    }

    constexpr void start(World& world) const noexcept {
        for (auto& func: startSystems) {
            func(world);
        }
    }

    constexpr void run(World& world) noexcept {
#ifdef ZERENGINE_USE_RENDER_THREAD
        if (renderCopyFunc != nullptr && renderFunc != nullptr) {
            renderThread.copyData(world, renderCopyFunc);
            renderThread.startRender(renderFunc);
        }
#endif

        for (const auto& mainFunc: mainSystems) {
            if (mainFunc.first == nullptr || mainFunc.first(world)) {
                mainFunc.second(world);
            }
        }

        for (const auto& funcs: systems) {
            if (funcs.first == nullptr || funcs.first(world)) {
                for (auto& func: funcs.second) {
                    threadpool.addTask(func);
                }
                threadpool.wait();
            }
        }

#ifdef ZERENGINE_USE_RENDER_THREAD
        if (renderCopyFunc != nullptr && renderFunc != nullptr) {
            renderThread.wait();
        }
#endif

        for (const auto& lateFuncs: lateSystems) {
            if (lateFuncs.first == nullptr || lateFuncs.first(world)) {
                for (auto& func: lateFuncs.second) {
                    threadpool.addTask(func);
                }
                threadpool.wait();
            }
        }
    }

public:
    std::vector<void(*)(World&)> startSystems;
    std::vector<std::pair<bool(*)(World&), void(*)(World&)>> mainSystems;
    std::vector<std::pair<bool(*)(World&), std::vector<void(*)(World&)>>> systems;
    std::vector<std::pair<bool(*)(World&), std::vector<void(*)(World&)>>> lateSystems;
    ThreadPool threadpool;

#ifdef ZERENGINE_USE_RENDER_THREAD
    void(*renderCopyFunc)(World&, LiteRegistry&);
    void(*renderFunc)(World&, LiteRegistry&);
    RenderThread renderThread;
#endif
};