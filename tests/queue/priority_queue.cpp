#include <gtest/gtest.h>

#include <chrono>
#include <future>

#include "queue/priority_queue.hpp"

using dispatcher::TaskPriority;
using dispatcher::queue::PriorityQueue;

TEST(PriorityQueueTest, HighPriorityTasksAreReturnedFirst) {
    PriorityQueue queue({{TaskPriority::High, {true, 2}}, {TaskPriority::Normal, {false, std::nullopt}}});
    int result = 0;
    queue.push(TaskPriority::Normal, [&] { result = 3; });
    queue.push(TaskPriority::High, [&] { result = 1; });
    queue.push(TaskPriority::High, [&] { result = 2; });
    queue.push(TaskPriority::Normal, [&] { result = 4; });
    queue.shutdown();

    for (int expected = 1; expected <= 4; ++expected) {
        auto task = queue.pop();
        ASSERT_TRUE(task.has_value());
        (*task)();
        EXPECT_EQ(result, expected);
    }
    EXPECT_FALSE(queue.pop().has_value());
}

TEST(PriorityQueueTest, EmptyQueueReturnsNulloptAfterShutdown) {
    PriorityQueue queue({{TaskPriority::Normal, {false, std::nullopt}}});
    queue.shutdown();
    EXPECT_FALSE(queue.pop().has_value());
}

TEST(PriorityQueueTest, PopWaitsForNewTask) {
    PriorityQueue queue({{TaskPriority::Normal, {false, std::nullopt}}});
    auto result = std::async(std::launch::async, [&] { return queue.pop(); });
    EXPECT_EQ(result.wait_for(std::chrono::milliseconds(20)), std::future_status::timeout);

    int value = 0;
    queue.push(TaskPriority::Normal, [&] { value = 42; });
    EXPECT_EQ(result.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    queue.shutdown();
    auto task = result.get();
    ASSERT_TRUE(task.has_value());
    (*task)();
    EXPECT_EQ(value, 42);
}

TEST(PriorityQueueTest, ShutdownWakesWaitingConsumer) {
    PriorityQueue queue({{TaskPriority::Normal, {false, std::nullopt}}});
    auto result = std::async(std::launch::async, [&] { return queue.pop(); });
    EXPECT_EQ(result.wait_for(std::chrono::milliseconds(20)), std::future_status::timeout);

    queue.shutdown();
    EXPECT_EQ(result.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    EXPECT_FALSE(result.get().has_value());
}

TEST(PriorityQueueTest, CanPushAfterPop) {
    PriorityQueue queue({{TaskPriority::High, {true, 1}}});
    queue.push(TaskPriority::High, [] {});
    ASSERT_TRUE(queue.pop().has_value());

    int value = 0;
    queue.push(TaskPriority::High, [&] { value = 42; });
    queue.shutdown();
    auto task = queue.pop();
    ASSERT_TRUE(task.has_value());
    (*task)();
    EXPECT_EQ(value, 42);
}
