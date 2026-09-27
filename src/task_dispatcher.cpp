#include "task_dispatcher.hpp"

#include <utility>

namespace dispatcher {

TaskDispatcher::TaskDispatcher(std::size_t thread_count,
                               const std::unordered_map<TaskPriority, queue::QueueOptions> &options)
    : queue_(std::make_shared<queue::PriorityQueue>(options)), pool_(queue_, thread_count) {}

void TaskDispatcher::schedule(TaskPriority priority, std::function<void()> task) {
    queue_->push(priority, std::move(task));
}

TaskDispatcher::~TaskDispatcher() = default;

}  // namespace dispatcher
