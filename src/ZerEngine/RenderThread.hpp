#pragma once

#ifdef ZERENGINE_USE_RENDER_THREAD

    #include <vector>
    #include <functional>
    #include <thread>
    #include <condition_variable>
    #include <mutex>
    #include <future>
    #include <queue>
    #include <semaphore>
    #include "View.hpp"
    #include "LiteRegistry.hpp"

    class World;

    class RenderThread final {
    public:
        RenderThread(World& newWorld) noexcept:
            world(newWorld),
            copyReg(new LiteRegistry()),
            renderTask(nullptr),
            semTask(0),
            semEnd(0),
            renderThread(std::bind(&RenderThread::task, this)),
            isStop(false) {
        }

        ~RenderThread() noexcept {
            stop();
            renderThread.join();
            delete copyReg;
        }

        constexpr void copyData(World& newWorld, void(*copyFunc)(World&, LiteRegistry&)) {
            copyFunc(newWorld, *copyReg);
        }

        constexpr void startRender(void(*func)(World&, LiteRegistry&)) noexcept {
            renderTask = func;
            semTask.release();
        }

        constexpr void wait() noexcept {
            semEnd.acquire();
        }

    private:
        void task() noexcept {
            while (true) {
                semTask.acquire();
                if (isStop) {
                    return;
                }

                renderTask(world, *copyReg);

                semEnd.release();
            }
        }

        constexpr void stop() noexcept {
            isStop = true;
            semTask.release();
        }

    private:
        World& world;
        LiteRegistry* copyReg;
        void(*renderTask)(World&, LiteRegistry&);
        std::binary_semaphore semTask;
        std::binary_semaphore semEnd;
        std::thread renderThread;
        std::atomic_bool isStop;
    };

#endif