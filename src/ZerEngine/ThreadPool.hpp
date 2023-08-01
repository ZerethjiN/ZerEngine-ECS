#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <future>
#include <queue>
#include <semaphore>
#include <functional>
#include "Archetype.hpp"

class World;

class IFuncObj {
public:
    virtual constexpr ~IFuncObj() noexcept = default;
    virtual void each() const noexcept = 0;
};

template <typename Func, typename... Comps>
class FuncObj final: public IFuncObj {
public:
    constexpr FuncObj(const Func& newFunc, const Archetype* newArch) noexcept:
        func(newFunc), arch(newArch) {
    }

    void each() const noexcept override final {
        for (const auto& pair: arch->entIdx) {
            if constexpr (std::is_invocable_v<Func, Comps&...>)
                std::apply(func, std::forward_as_tuple(arch->getAt<Comps>(pair.second)...));
            else if constexpr (std::is_invocable_v<Func, Ent, Comps&...>)
                std::apply(func, std::forward_as_tuple(pair.first, arch->getAt<Comps>(pair.second)...));
        }
    }

private:
    const Func& func;
    const Archetype* arch;
};

class ThreadPool final {
public:
    inline ThreadPool(World& newWorld, std::size_t newNbThreads) noexcept:
        world(newWorld),
        nbTasks(0),
        nbQueryTasks(0),
        nbThreads(newNbThreads),
        isStop(false) {
        for (std::size_t i = 0; i < nbThreads; i++) {
            threads.emplace_back(std::bind(&ThreadPool::task, this));
        }
    }

    inline ~ThreadPool() noexcept {
        stop();
        for (auto& thread: threads) {
            thread.join();
        }
    }

    inline void addTask(void(*const func)(World&)) noexcept {
        std::lock_guard<std::mutex> lock(mtx);
        tasks.emplace_back(func);
    }

    inline bool canQueryTask() noexcept {
        return !(nbTasks >= nbThreads || !tasks.empty());
    }

    template <typename Func, typename... Comps>
    inline void addQueryTask(const Func& newFunc, const Archetype* newArch) noexcept {
        std::lock_guard<std::mutex> lock(mtx);
        queryTasks.emplace_back(new FuncObj<Func, Comps...>(newFunc, newArch));
    }

    inline void wait() noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        cvTask.notify_all();
        cvFinished.wait(lock, [&]() {
            return tasks.empty() && (nbTasks == 0);
        });
    }

    inline void waitQueryTask() noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        cvTask.notify_all();
        cvQueryFinished.wait(lock, [&]() {
            return queryTasks.empty() && (nbQueryTasks == 0);
        });
    }

private:
    void task() noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        while (true) {
            cvTask.wait(lock, [&]() {
                return (nbTasks < nbThreads) && (!tasks.empty() || !queryTasks.empty() || isStop);
            });
            if (isStop && tasks.empty() && queryTasks.empty()) {
                return;
            }

            if (!tasks.empty()) {
                nbTasks++;
                auto newTask = tasks.back();
                tasks.pop_back();
                lock.unlock();

                newTask(world);

                lock.lock();
                nbTasks--;
                cvFinished.notify_all();
            } else if (!queryTasks.empty()) {
                nbTasks++;
                nbQueryTasks++;
                auto* newTask = queryTasks.back();
                queryTasks.pop_back();
                lock.unlock();

                newTask->each();

                lock.lock();
                delete newTask;
                nbQueryTasks--;
                nbTasks--;
                cvQueryFinished.notify_all();
            }
        }
    }

    inline void stop() noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        isStop = true;
        cvTask.notify_all();
    }

private:
    World& world;
    std::vector<void(*)(World&)> tasks;
    std::vector<IFuncObj*> queryTasks;
    std::mutex mtx;
    std::condition_variable cvTask;
    std::condition_variable cvFinished;
    std::condition_variable cvQueryFinished;
    std::vector<std::thread> threads;
    std::atomic_size_t nbTasks;
    std::atomic_size_t nbQueryTasks;
    std::atomic_size_t nbThreads;
    std::atomic_bool isStop;
};