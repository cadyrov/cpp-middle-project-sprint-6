#pragma once

#include "queue/priority_queue.hpp"

#include <cstddef>
#include <memory>
#include <thread>
#include <vector>

namespace dispatcher::thread_pool {

class ThreadPool {
public:
    ThreadPool(std::shared_ptr<queue::PriorityQueue> queue, std::size_t thread_count);
    ~ThreadPool();

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

private:
    void worker();

    std::shared_ptr<queue::PriorityQueue> queue_;
    std::vector<std::thread> threads_;
};

}  // namespace dispatcher::thread_pool
