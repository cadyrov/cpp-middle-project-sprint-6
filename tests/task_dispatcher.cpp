#include <gtest/gtest.h>

#include <atomic>
#include <future>
#include <stdexcept>
#include <vector>

#include "task_dispatcher.hpp"

using dispatcher::TaskDispatcher;
using dispatcher::TaskPriority;

TEST(TaskDispatcherTest, ExecutesTasksWithBothPriorities) {
    std::atomic<int> high_count{0};
    std::atomic<int> normal_count{0};
    {
        TaskDispatcher dispatcher(2);
        dispatcher.schedule(TaskPriority::High, [&] { ++high_count; });
        dispatcher.schedule(TaskPriority::Normal, [&] { ++normal_count; });
    }

    EXPECT_EQ(high_count.load(), 1);
    EXPECT_EQ(normal_count.load(), 1);
}

TEST(TaskDispatcherTest, DestructorWaitsForAllTasks) {
    std::atomic<int> completed{0};
    {
        TaskDispatcher dispatcher(2);
        for (int i = 0; i < 100; ++i) {
            dispatcher.schedule(TaskPriority::Normal, [&] { ++completed; });
        }
    }

    EXPECT_EQ(completed.load(), 100);
}

TEST(TaskDispatcherTest, AcceptsCustomQueueConfiguration) {
    std::atomic<int> completed{0};
    {
        TaskDispatcher dispatcher(1, {{TaskPriority::High, {false, std::nullopt}}, {TaskPriority::Normal, {true, 5}}});
        dispatcher.schedule(TaskPriority::High, [&] { ++completed; });
        for (int i = 0; i < 5; ++i) {
            dispatcher.schedule(TaskPriority::Normal, [&] { ++completed; });
        }
    }

    EXPECT_EQ(completed.load(), 6);
}

TEST(TaskDispatcherTest, SingleWorkerPreservesPriorityAndInsertionOrder) {
    std::vector<int> order;
    std::promise<void> release;
    auto ready = release.get_future();
    {
        TaskDispatcher dispatcher(1);
        // Занимаем единственный поток, пока добавляем проверяемые задачи.
        dispatcher.schedule(TaskPriority::High, [&] { ready.wait(); });
        dispatcher.schedule(TaskPriority::Normal, [&] { order.push_back(3); });
        dispatcher.schedule(TaskPriority::High, [&] { order.push_back(1); });
        dispatcher.schedule(TaskPriority::Normal, [&] { order.push_back(4); });
        dispatcher.schedule(TaskPriority::High, [&] { order.push_back(2); });
        release.set_value();
    }
    EXPECT_EQ(order, (std::vector<int>{1, 2, 3, 4}));
}

TEST(TaskDispatcherTest, ContinuesAfterTaskExceptions) {
    int completed = 0;
    {
        TaskDispatcher dispatcher(1);
        dispatcher.schedule(TaskPriority::Normal, [] { throw std::runtime_error("Test error"); });
        dispatcher.schedule(TaskPriority::Normal, [&] { ++completed; });
        dispatcher.schedule(TaskPriority::Normal, [] { throw 42; });
        dispatcher.schedule(TaskPriority::Normal, [&] { ++completed; });
    }
    EXPECT_EQ(completed, 2);
}
