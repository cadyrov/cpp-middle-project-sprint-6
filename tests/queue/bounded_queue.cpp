#include <gtest/gtest.h>

#include "queue/bounded_queue.hpp"

using dispatcher::queue::BoundedQueue;

TEST(BoundedQueueTest, EmptyQueueReturnsNullopt) {
    BoundedQueue queue(2);
    EXPECT_FALSE(queue.try_pop().has_value());
}

TEST(BoundedQueueTest, TasksAreReturnedInInsertionOrder) {
    BoundedQueue queue(2);
    int result = 0;
    queue.push([&] { result = 1; });
    queue.push([&] { result = 2; });

    for (int expected = 1; expected <= 2; ++expected) {
        auto task = queue.try_pop();
        ASSERT_TRUE(task.has_value());
        (*task)();
        EXPECT_EQ(result, expected);
    }
    EXPECT_FALSE(queue.try_pop().has_value());
}

TEST(BoundedQueueTest, CanPushAfterPop) {
    BoundedQueue queue(1);
    queue.push([] {});
    ASSERT_TRUE(queue.try_pop().has_value());

    int result = 0;
    queue.push([&] { result = 42; });
    auto task = queue.try_pop();
    ASSERT_TRUE(task.has_value());
    (*task)();
    EXPECT_EQ(result, 42);
}

TEST(BoundedQueueTest, PushDoesNotExecuteTask) {
    BoundedQueue queue(1);
    bool executed = false;
    queue.push([&] { executed = true; });
    EXPECT_FALSE(executed);
}

TEST(BoundedQueueTest, PoppedTaskCanBeExecutedAfterQueueDestruction) {
    std::optional<std::function<void()>> task;
    int result = 0;
    {
        BoundedQueue queue(1);
        queue.push([&] { result = 42; });
        task = queue.try_pop();
    }
    ASSERT_TRUE(task.has_value());
    (*task)();
    EXPECT_EQ(result, 42);
}
