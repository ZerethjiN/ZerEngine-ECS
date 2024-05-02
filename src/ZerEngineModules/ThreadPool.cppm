module;

#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <semaphore>
#include <functional>

export module ZerengineCore:ThreadPool;

class World;

class ThreadPool final {
friend class Sys;
private:
    ThreadPool(World& newWorld, std::size_t newNbThreads) noexcept:
        world(newWorld),
        nbTasksDone(0),
        nbTasks(0),
        nbThreads(newNbThreads),
        isStop(false) {
        for (std::size_t i = 0; i < nbThreads; i++) {
            threads.emplace_back(std::bind(&ThreadPool::task, this));
        }
    }

private:
    ~ThreadPool() noexcept {
        stop();
        for (auto& thread: threads) {
            thread.join();
        }
    }

    void addTasks(const std::vector<void(*)(World&)>& newTasks) noexcept {
        tasks.emplace_back(newTasks);
    }

    void run() noexcept {
        if (!tasks.empty()) {
            nbTasksDone = tasks[0].size();
            cvTask.notify_all();
        } else {
            nbTasksDone = 0;
        }
    }

    void wait() noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        cvFinished.wait(lock, [&]() {
            if (!tasks.empty() && nbTasksDone != 0) {
               cvTask.notify_all(); 
            }
            return (tasks.empty() && (nbTasks == 0));
        });
    }

private:
    void task() noexcept {
        srand(time(NULL));
        std::unique_lock<std::mutex> lock(mtx);
        while (true) {
            cvTask.wait(lock, [&]() {
                return (nbTasks < nbThreads) && ((!tasks.empty() && nbTasksDone != 0) || isStop);
            });
            if (isStop && tasks.empty()) {
                return;
            }

            nbTasks++;
            auto newTask = tasks[0].back();
            tasks[0].pop_back();
            nbTasksDone--;
            lock.unlock();

            newTask(world);

            lock.lock();
            nbTasks--;

            if (nbTasksDone == 0 && nbTasks == 0) {
                tasks.erase(tasks.begin());
                if (!tasks.empty()) {
                    nbTasksDone = tasks[0].size();
                    cvFinished.notify_one();
                }
            }

            if (tasks.empty() && nbTasks == 0) {
                cvFinished.notify_one();
            }
        }
    }

    void stop() noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        isStop = true;
        cvTask.notify_all();
    }

private:
    World& world;
    std::vector<std::vector<void(*)(World&)>> tasks;
    std::mutex mtx;
    std::size_t nbTasksDone;
    std::condition_variable cvTask;
    std::condition_variable cvFinished;
    std::vector<std::thread> threads;
    std::size_t nbTasks;
    std::size_t nbThreads;
    bool isStop;
};