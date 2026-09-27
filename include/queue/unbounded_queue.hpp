#pragma once
#include "queue/queue.hpp"

#include <mutex>
#include <queue>

namespace dispatcher::queue {

class UnboundedQueue : public IQueue {
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;

public:
    // capacity сохраняется для совместимости интерфейса и не ограничивает очередь.
    explicit UnboundedQueue(int capacity);

    void push(std::function<void()> task) override;

    std::optional<std::function<void()>> try_pop() override;

    ~UnboundedQueue() override;
};

}  // namespace dispatcher::queue
