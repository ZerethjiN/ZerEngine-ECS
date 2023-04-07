#pragma once

#include <vector>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <future>
#include <queue>
#include <semaphore>
#include "../Registry/LiteRegistry.hpp"

namespace zre {
    class World;

    namespace priv {
        class RenderThread {
        public:
            inline RenderThread(World& newWorld) noexcept:
                world(newWorld),
                renderTask(nullptr),
                semTask(0),
                semEnd(0),
                renderThread(std::bind(&RenderThread::task, this)),
                isStop(false) {
            }

            inline ~RenderThread() noexcept {
                stop();
                renderThread.join();
            }

            constexpr void copyData(World& newWorld, void(*copyFunc)(World&, LiteRegistry&)) {
                copyFunc(newWorld, copyReg);
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

                    renderTask(world, copyReg);

                    semEnd.release();
                }
            }

            constexpr void stop() noexcept {
                isStop = true;
                semTask.release();
            }

        private:
            World& world;
            LiteRegistry copyReg;
            void(*renderTask)(World&, LiteRegistry&);
            std::binary_semaphore semTask;
            std::binary_semaphore semEnd;
            std::thread renderThread;
            std::atomic_bool isStop;
        };
    }
}