/**
 * @file Threadpool.hpp
 * @author ZerethjiN
 * @brief Multithreaded systems managed and allocated by a threadpool.
 * @version 0.1
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022 - ZerethjiN
 */
#ifndef ZERENGINE_THREADPOOL_HPP
#define ZERENGINE_THREADPOOL_HPP

#include <vector>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <future>
#include <queue>

namespace zre {
    struct World;
    namespace priv {
        class ThreadPool {
        public:
            /**
             * @brief Construct a new Thread Pool object and allocates its threads.
             * 
             * @param newWorld 
             */
            ThreadPool(World& newWorld):
                world(newWorld),
                nbThreads(std::thread::hardware_concurrency()) {
                for (unsigned int i = 0; i < std::thread::hardware_concurrency() - 1; i++) {
                    threads.emplace_back(std::bind(&ThreadPool::task, this));
                }
            }

            ~ThreadPool() {
                stop();
                for (auto& thread: threads) {
                    thread.join();
                }
            }

            template <class Func>
            void addTask(Func&& func) {
                std::unique_lock<std::mutex> lock(mtx);
                tasks.emplace(std::forward<Func>(func));
                cvTask.notify_one();
            }

            void wait() {
                std::unique_lock<std::mutex> lock(mtx);
                cvFinished.wait(lock, [&]() {
                    return tasks.empty() && (busy == 0);
                });
            }

        private:
            void stop() {
                std::unique_lock<std::mutex> lock(mtx);
                isStop = true;
                cvTask.notify_all();
                lock.unlock();
            }

            void task() {
                while (!isStop) {
                    std::unique_lock<std::mutex> lock(mtx);
                    cvTask.wait(lock, [&]() {
                        return (!tasks.empty() || isStop) && busy < nbThreads;
                    });
                    if (isStop && tasks.empty()) {
                        return;
                    }
                    busy++;
                    auto task = std::move(tasks.front());
                    tasks.pop();
                    lock.unlock();

                    task(world);

                    lock.lock();
                    busy--;
                    cvFinished.notify_one();
                }
            }

        private:
            World& world;
            std::queue<std::function<void(zre::World&)>> tasks;
            std::mutex mtx;
            std::condition_variable cvTask;
            std::condition_variable cvFinished;
            std::atomic_bool isStop = false;
            std::vector<std::thread> threads;
            unsigned int busy;
            unsigned int nbThreads;
        };
    }
}

#endif