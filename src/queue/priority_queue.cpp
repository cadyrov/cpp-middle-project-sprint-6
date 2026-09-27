#include "queue/priority_queue.hpp"

#include <stdexcept>
#include <utility>

namespace dispatcher::queue {

PriorityQueue::PriorityQueue(const std::unordered_map<TaskPriority, QueueOptions> &options) {
    for (const auto &[priority, config] : options) {
        if (config.bounded) {
            if (!config.capacity) {
                throw std::invalid_argument("Bounded queue requires a capacity");
            }
            queues_.emplace(priority, std::make_unique<BoundedQueue>(*config.capacity));
        } else {
            queues_.emplace(priority, std::make_unique<UnboundedQueue>(config.capacity.value_or(0)));
        }
    }
}

void PriorityQueue::push(TaskPriority priority, std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queues_.at(priority)->push(std::move(task));
    }
    condition_.notify_one();
}

std::optional<std::function<void()>> PriorityQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (true) {
        // TaskPriority::High precedes TaskPriority::Normal in the ordered map.
        for (auto &[priority, queue] : queues_) {
            if (auto task = queue->try_pop()) {
                return task;
            }
        }
        if (shutdown_) {
            return std::nullopt;
        }
        condition_.wait(lock);
    }
}

void PriorityQueue::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
    }
    condition_.notify_all();
}

PriorityQueue::~PriorityQueue() = default;

}  // namespace dispatcher::queue
