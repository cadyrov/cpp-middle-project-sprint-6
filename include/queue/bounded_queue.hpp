#pragma once
#include "queue/queue.hpp"

#include <cstddef>
#include <mutex>
#include <queue>

namespace dispatcher::queue {

class BoundedQueue : public IQueue {
    const std::size_t capacity_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;

public:
    explicit BoundedQueue(int capacity);

    void push(std::function<void()> task) override;

    std::optional<std::function<void()>> try_pop() override;

    ~BoundedQueue() override;
};

}  // namespace dispatcher::queue
