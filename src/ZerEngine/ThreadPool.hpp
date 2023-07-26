#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <future>
#include <queue>
#include <semaphore>
#include <functional>

class World;

class ThreadPool final {
public:
    inline ThreadPool(World& newWorld, std::size_t newNbThreads) noexcept:
        world(newWorld),
        nbTasks(0),
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

    inline void wait() noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        cvTask.notify_all();
        cvFinished.wait(lock, [&]() {
            return tasks.empty() && (nbTasks == 0);
        });
    }

private:
    void task() noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        while (true) {
            cvTask.wait(lock, [&]() {
                return (nbTasks < nbThreads) && (!tasks.empty() || isStop);
            });
            if (isStop && tasks.empty()) {
                return;
            }

            nbTasks++;
            auto newTask = tasks.back();
            tasks.pop_back();
            lock.unlock();

            newTask(world);

            lock.lock();
            nbTasks--;
            cvFinished.notify_all();
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
    std::mutex mtx;
    std::condition_variable cvTask;
    std::condition_variable cvFinished;
    std::vector<std::thread> threads;
    std::atomic_size_t nbTasks;
    std::atomic_size_t nbThreads;
    std::atomic_bool isStop;
};