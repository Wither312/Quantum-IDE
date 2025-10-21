#pragma once

#include <functional>
#include <future>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace core {

    class ThreadPool {
    public:
        ThreadPool() {}
     //   explicit ThreadPool(std::size_t threadCount = std::thread::hardware_concurrency());
        ~ThreadPool(){};

        template<typename Func, typename... Args>
        auto enqueue(Func&& func, Args&&... args) -> std::future<std::invoke_result_t<Func, Args...>>;

        void waitIdle();

    private:
        void workerLoop();

        std::vector<std::thread> m_workers;
        std::queue<std::function<void()>> m_tasks;

        std::mutex m_mutex;
        std::condition_variable m_condition;
        std::atomic<bool> m_stop{ false };
    };

} // namespace core
