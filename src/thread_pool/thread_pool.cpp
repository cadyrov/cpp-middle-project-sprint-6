#include "thread_pool/thread_pool.hpp"

#include "logger.hpp"

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

namespace dispatcher::thread_pool {

ThreadPool::ThreadPool(std::shared_ptr<queue::PriorityQueue> queue, std::size_t thread_count)
    : queue_(std::move(queue)) {
    if (!queue_) {
        throw std::invalid_argument("Thread pool requires a queue");
    }
    if (thread_count == 0) {
        throw std::invalid_argument("Thread count must be positive");
    }

    threads_.reserve(thread_count);
    try {
        for (std::size_t i = 0; i < thread_count; ++i) {
            threads_.emplace_back(&ThreadPool::worker, this);
        }
    } catch (...) {
        queue_->shutdown();
        for (auto &thread : threads_) {
            thread.join();
        }
        throw;
    }
}

ThreadPool::~ThreadPool() {
    queue_->shutdown();
    for (auto &thread : threads_) {
        thread.join();
    }
}

void ThreadPool::worker() {
    while (auto task = queue_->pop()) {
        try {
            (*task)();
        } catch (const std::exception &error) {
            Logger::Get().Log(std::string("Task failed: ") + error.what());
        } catch (...) {
            Logger::Get().Log("Task failed: unknown exception");
        }
    }
}

}  // namespace dispatcher::thread_pool
