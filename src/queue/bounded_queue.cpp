#include "queue/bounded_queue.hpp"

#include <stdexcept>
#include <utility>

namespace dispatcher::queue {

BoundedQueue::BoundedQueue(int capacity) : capacity_(capacity) {
    if (capacity <= 0) {
        throw std::invalid_argument("Queue capacity must be positive");
    }
}

void BoundedQueue::push(std::function<void()> task) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (tasks_.size() >= capacity_) {
        throw std::overflow_error("Queue is full");
    }
    tasks_.push(std::move(task));
}

std::optional<std::function<void()>> BoundedQueue::try_pop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (tasks_.empty()) {
        return std::nullopt;
    }

    auto task = std::move(tasks_.front());
    tasks_.pop();
    return task;
}

BoundedQueue::~BoundedQueue() = default;

}  // namespace dispatcher::queue
