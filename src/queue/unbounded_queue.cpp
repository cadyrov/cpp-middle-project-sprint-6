#include "queue/unbounded_queue.hpp"

#include <utility>

namespace dispatcher::queue {

UnboundedQueue::UnboundedQueue(int /*capacity*/) {}

void UnboundedQueue::push(std::function<void()> task) {
    std::lock_guard<std::mutex> lock(mutex_);
    tasks_.push(std::move(task));
}

std::optional<std::function<void()>> UnboundedQueue::try_pop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (tasks_.empty()) {
        return std::nullopt;
    }

    auto task = std::move(tasks_.front());
    tasks_.pop();
    return task;
}

UnboundedQueue::~UnboundedQueue() = default;

}  // namespace dispatcher::queue
