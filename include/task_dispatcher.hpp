#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

#include "queue/priority_queue.hpp"
#include "thread_pool/thread_pool.hpp"
#include "types.hpp"

namespace dispatcher {

class TaskDispatcher {
    std::shared_ptr<queue::PriorityQueue> queue_;
    thread_pool::ThreadPool pool_;

public:
    explicit TaskDispatcher(std::size_t thread_count,
                            const std::unordered_map<TaskPriority, queue::QueueOptions> &options = {
                                {TaskPriority::High, {true, 1000}}, {TaskPriority::Normal, {false, std::nullopt}}});

    void schedule(TaskPriority priority, std::function<void()> task);
    ~TaskDispatcher();
};

}  // namespace dispatcher
