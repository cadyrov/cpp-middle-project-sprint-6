#include <gtest/gtest.h>

#include "queue/unbounded_queue.hpp"

using dispatcher::queue::UnboundedQueue;

TEST(UnboundedQueueTest, EmptyQueueReturnsNullopt) {
    UnboundedQueue queue(0);
    EXPECT_FALSE(queue.try_pop().has_value());
}

TEST(UnboundedQueueTest, TasksAreReturnedInInsertionOrderWithoutCapacityLimit) {
    UnboundedQueue queue(1);
    int result = 0;
    for (int value = 1; value <= 3; ++value) {
        queue.push([&, value] { result = value; });
    }

    for (int expected = 1; expected <= 3; ++expected) {
        auto task = queue.try_pop();
        ASSERT_TRUE(task.has_value());
        (*task)();
        EXPECT_EQ(result, expected);
    }
    EXPECT_FALSE(queue.try_pop().has_value());
}

TEST(UnboundedQueueTest, CanPushAfterPop) {
    UnboundedQueue queue(0);
    queue.push([] {});
    ASSERT_TRUE(queue.try_pop().has_value());

    int result = 0;
    queue.push([&] { result = 42; });
    auto task = queue.try_pop();
    ASSERT_TRUE(task.has_value());
    (*task)();
    EXPECT_EQ(result, 42);
}

TEST(UnboundedQueueTest, PushDoesNotExecuteTask) {
    UnboundedQueue queue(0);
    bool executed = false;
    queue.push([&] { executed = true; });
    EXPECT_FALSE(executed);
}

TEST(UnboundedQueueTest, PoppedTaskCanBeExecutedAfterQueueDestruction) {
    std::optional<std::function<void()>> task;
    int result = 0;
    {
        UnboundedQueue queue(0);
        queue.push([&] { result = 42; });
        task = queue.try_pop();
    }
    ASSERT_TRUE(task.has_value());
    (*task)();
    EXPECT_EQ(result, 42);
}
