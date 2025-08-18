#include <atomic>
#include <chrono>
#include <future>
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "TaskScheduler.h"

using namespace std::chrono_literals;

class TaskSchedulerTest : public ::testing::Test
{
protected:
    void SetUp () override { scheduler = std::make_unique<TaskScheduler> (); }

    void TearDown () override
    {
        if (scheduler && scheduler->IsRunning ())
        {
            scheduler->Stop (TaskScheduler::ShutdownMode::IMMEDIATE);
        }
        scheduler.reset ();
    }

    std::unique_ptr<TaskScheduler> scheduler;    
};

// Test fixture for running scheduler tests
class RunningTaskSchedulerTest : public TaskSchedulerTest
{
protected:
    void SetUp () override
    {
        TaskSchedulerTest::SetUp ();
        scheduler->Start ();
        // Give scheduler time to start
        std::this_thread::sleep_for (10ms);
    }
};

// ============ Basic Functionality Tests ============

TEST_F (TaskSchedulerTest, DefaultConstructorCreatesStoppedScheduler)
{
    EXPECT_FALSE (scheduler->IsRunning ());
    EXPECT_EQ (scheduler->GetTaskCount (), 0);
}

TEST_F (TaskSchedulerTest, StartSchedulerChangesRunningState)
{
    EXPECT_FALSE (scheduler->IsRunning ());

    scheduler->Start ();
    EXPECT_TRUE (scheduler->IsRunning ());

    scheduler->Stop ();
    EXPECT_FALSE (scheduler->IsRunning ());
}

TEST_F (TaskSchedulerTest, StartAlreadyRunningSchedulerDoesNothing)
{
    scheduler->Start ();
    EXPECT_TRUE (scheduler->IsRunning ());

    // Starting again should not cause issues
    EXPECT_NO_THROW (scheduler->Start ());
    EXPECT_TRUE (scheduler->IsRunning ());
}

TEST_F (TaskSchedulerTest, StopNonRunningSchedulerDoesNothing)
{
    EXPECT_FALSE (scheduler->IsRunning ());
    EXPECT_NO_THROW (scheduler->Stop ());
    EXPECT_FALSE (scheduler->IsRunning ());
}

// ============ Task Addition Tests ============

TEST_F (RunningTaskSchedulerTest, AddTaskReturnsValidTaskID)
{
    std::atomic<bool> executed {false};

    TaskID id = scheduler->AddTask ([&executed] () { executed = true; }, 50ms);

    EXPECT_GT (id, 0);

    std::this_thread::sleep_for (100ms);
    EXPECT_TRUE (executed);
}

TEST_F (RunningTaskSchedulerTest, AddTaskIncreasesTaskCount)
{
    EXPECT_EQ (scheduler->GetTaskCount (), 0);

    scheduler->AddTask ([] () { }, 100ms);
    EXPECT_EQ (scheduler->GetTaskCount (), 1);

    scheduler->AddTask ([] () { }, 200ms);
    EXPECT_EQ (scheduler->GetTaskCount (), 2);
}

TEST_F (RunningTaskSchedulerTest, AddMultipleTasksGetUniqueIDs)
{
    TaskID id1 = scheduler->AddTask ([] () { }, 100ms);
    TaskID id2 = scheduler->AddTask ([] () { }, 100ms);
    TaskID id3 = scheduler->AddTask ([] () { }, 100ms);

    EXPECT_NE (id1, id2);
    EXPECT_NE (id2, id3);
    EXPECT_NE (id1, id3);
}

// ============ Task Execution Tests ============

TEST_F (RunningTaskSchedulerTest, ImmediateTaskExecutesQuickly)
{
    std::atomic<bool> executed {false};
    auto start = std::chrono::steady_clock::now ();

    scheduler->AddTask ([&executed] () { executed = true; }, 0ms);

    // Wait for execution
    std::this_thread::sleep_for (50ms);

    auto elapsed = std::chrono::steady_clock::now () - start;
    EXPECT_TRUE (executed);
    EXPECT_LT (elapsed, 100ms); // Should execute quickly
}

TEST_F (RunningTaskSchedulerTest, DelayedTaskExecutesAfterDelay)
{
    std::atomic<bool> executed{false};
    
    scheduler->AddTask([&executed]() { 
        executed = true;
    }, 100ms);
    
    // Check it hasn't executed immediately
    std::this_thread::sleep_for(50ms);
    EXPECT_FALSE(executed);
    
    // Wait for execution
    std::this_thread::sleep_for(100ms);
    EXPECT_TRUE(executed);
}

TEST_F (RunningTaskSchedulerTest, MultipleTasksExecuteInOrder)
{
    std::vector<int> execution_order;
    std::mutex order_mutex;

    // Add tasks with different delays
    scheduler->AddTask (
            [&execution_order, &order_mutex] ()
            {
                std::lock_guard<std::mutex> lock (order_mutex);
                execution_order.push_back (1);
            },
            10ms);

    scheduler->AddTask (
            [&execution_order, &order_mutex] ()
            {
                std::lock_guard<std::mutex> lock (order_mutex);
                execution_order.push_back (2);
            },
            20ms);

    scheduler->AddTask (
            [&execution_order, &order_mutex] ()
            {
                std::lock_guard<std::mutex> lock (order_mutex);
                execution_order.push_back (3);
            },
            30ms);

    // Wait for all tasks to complete
    std::this_thread::sleep_for (100ms);

    ASSERT_EQ (execution_order.size (), 3);
    EXPECT_EQ (execution_order[0], 1);
    EXPECT_EQ (execution_order[1], 2);
    EXPECT_EQ (execution_order[2], 3);
}

// ============ Recurring Task Tests ============

TEST_F (RunningTaskSchedulerTest, RecurringTaskExecutesMultipleTimes)
{
    std::atomic<int> execution_count {0};

    scheduler->AddTask (
            [&execution_count] () { execution_count.fetch_add (1); }, 10ms, 50ms); // Initial delay 10ms, interval 50ms

    // Wait for multiple executions
    std::this_thread::sleep_for (200ms);

    // Should have executed at least 3 times (10ms + 50ms + 50ms + 50ms = 160ms)
    EXPECT_GE (execution_count.load (), 3);
    EXPECT_LE (execution_count.load (), 5); // But not too many due to timing
}

TEST_F (RunningTaskSchedulerTest, RecurringTaskMaintainsInterval)
{
    std::vector<std::chrono::steady_clock::time_point> execution_times;
    std::mutex times_mutex;

    scheduler->AddTask (
            [&execution_times, &times_mutex] ()
            {
                std::lock_guard<std::mutex> lock (times_mutex);
                execution_times.push_back (std::chrono::steady_clock::now ());
            },
            10ms, 100ms);

    // Wait for multiple executions
    std::this_thread::sleep_for (350ms);

    // Check intervals
    ASSERT_GE (execution_times.size (), 2);
    for (size_t i = 1; i < execution_times.size (); ++i)
    {
        auto interval = execution_times[i] - execution_times[i - 1];
        auto interval_ms = std::chrono::duration_cast<std::chrono::milliseconds> (interval);

        // Allow some tolerance for timing variations
        EXPECT_GE (interval_ms.count (), 90); // At least 90ms
        EXPECT_LE (interval_ms.count (), 110); // At most 110ms
    }
}

// ============ Task Count Tests ============

TEST_F (RunningTaskSchedulerTest, TaskCountDecreasesAfterExecution)
{
    scheduler->AddTask ([] () { }, 50ms);
    scheduler->AddTask ([] () { }, 100ms);

    EXPECT_EQ (scheduler->GetTaskCount (), 2);

    std::this_thread::sleep_for (75ms);
    EXPECT_EQ (scheduler->GetTaskCount (), 1); // One should have executed

    std::this_thread::sleep_for (50ms);
    EXPECT_EQ (scheduler->GetTaskCount (), 0); // Both should have executed
}

TEST_F (RunningTaskSchedulerTest, RecurringTaskMaintainsCountInQueue)
{
    scheduler->AddTask ([] () { }, 10ms, 100ms);

    // Initially should have 1 task
    EXPECT_EQ (scheduler->GetTaskCount (), 1);

    // After first execution, should still have 1 (rescheduled)
    std::this_thread::sleep_for (50ms);
    EXPECT_EQ (scheduler->GetTaskCount (), 1);

    // Should maintain the count
    std::this_thread::sleep_for (100ms);
    EXPECT_EQ (scheduler->GetTaskCount (), 1);
}

// ============ Exception Handling Tests ============

TEST_F (RunningTaskSchedulerTest, TaskExceptionDoesNotCrashScheduler)
{
    std::atomic<bool> second_task_executed {false};

    // Add a task that throws
    scheduler->AddTask ([] () { throw std::runtime_error ("Test exception"); }, 10ms);

    // Add a task that should still execute
    scheduler->AddTask ([&second_task_executed] () { second_task_executed = true; }, 50ms);

    std::this_thread::sleep_for (100ms);

    // Scheduler should still be running and second task should execute
    EXPECT_TRUE (scheduler->IsRunning ());
    EXPECT_TRUE (second_task_executed);
}

// ============ Shutdown Tests ============

TEST_F (RunningTaskSchedulerTest, ImmediateShutdownStopsQuickly)
{
    // Add a long-running task
    scheduler->AddTask ([] () { std::this_thread::sleep_for (200ms); }, 10ms);

    auto start = std::chrono::steady_clock::now ();
    scheduler->Stop (TaskScheduler::ShutdownMode::IMMEDIATE);
    auto elapsed = std::chrono::steady_clock::now () - start;

    EXPECT_FALSE (scheduler->IsRunning ());
    EXPECT_LT (elapsed, 100ms); // Should stop quickly
}

TEST_F (RunningTaskSchedulerTest, GracefulShutdownWaitsForCurrentTasks)
{
    // Track task execution states
    std::atomic<bool> task1_started{false};
    std::atomic<bool> task1_completed{false};
    std::atomic<bool> task2_started{false};
    std::atomic<bool> task2_completed{false};
    
    // Add a long-running task that will be executing when Stop() is called
    auto task1_id = scheduler->AddTask([&]() {
        task1_started = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        task1_completed = true;
    }, std::chrono::milliseconds(10));
    
    // Add another task that should also complete during graceful shutdown
    auto task2_id = scheduler->AddTask([&]() {
        task2_started = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        task2_completed = true;
    }, std::chrono::milliseconds(50));
    
    // Wait for tasks to start executing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Verify tasks have started
    EXPECT_TRUE(task1_started.load());
    EXPECT_TRUE(task2_started.load());
    EXPECT_FALSE(task1_completed.load());
    EXPECT_FALSE(task2_completed.load());
    
    // Record shutdown start time
    auto shutdown_start = std::chrono::steady_clock::now();
    
    // Initiate graceful shutdown (should wait for current tasks to complete)
    scheduler->Stop(TaskScheduler::ShutdownMode::COMPLETE_CURRENT);
    
    // Record shutdown completion time
    auto shutdown_end = std::chrono::steady_clock::now();
    auto shutdown_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        shutdown_end - shutdown_start);
    
    // Verify that graceful shutdown waited for tasks to complete
    EXPECT_TRUE(task1_completed.load());
    EXPECT_TRUE(task2_completed.load());
    
    // Verify shutdown took reasonable time (at least as long as the longest task)
    // Task1 takes ~500ms, so shutdown should take at least that long
    EXPECT_GE(shutdown_duration.count(), 400); // Allow some tolerance
    
    // Verify scheduler is no longer running
    EXPECT_FALSE(scheduler->IsRunning());
    
    // Verify no tasks remain in queue
    EXPECT_EQ(scheduler->GetTaskCount(), 0);

}

// ============ Stress Tests ============

TEST_F (RunningTaskSchedulerTest, HandlesManySimultaneousTasks)
{
    const int NUM_TASKS = 100;
    std::atomic<int> completed_tasks {0};

    for (int i = 0; i < NUM_TASKS; ++i)
    {
        scheduler->AddTask ([&completed_tasks] () { completed_tasks.fetch_add (1); },
                std::chrono::milliseconds (i % 50)); // Spread out execution times
    }

    // Wait for all tasks to complete
    std::this_thread::sleep_for (200ms);

    EXPECT_EQ (completed_tasks.load (), NUM_TASKS);
    EXPECT_EQ (scheduler->GetTaskCount (), 0);
}

TEST_F (RunningTaskSchedulerTest, HandlesRapidTaskAddition)
{
    std::atomic<int> execution_count {0};

    // Add tasks rapidly
    for (int i = 0; i < 50; ++i)
    {
        scheduler->AddTask ([&execution_count] () { execution_count.fetch_add (1); }, 10ms);
    }

    std::this_thread::sleep_for (100ms);

    EXPECT_EQ (execution_count.load (), 50);
}

// ============ Edge Cases ============

TEST_F (RunningTaskSchedulerTest, ZeroDelayTaskExecutesImmediately)
{
    std::atomic<bool> executed {false};

    scheduler->AddTask ([&executed] () { executed = true; }, 0ms);

    // Very short wait should be enough
    std::this_thread::sleep_for (20ms);
    EXPECT_TRUE (executed);
}

TEST_F (RunningTaskSchedulerTest, VeryShortIntervalRecurringTask)
{
    std::atomic<int> count {0};

    scheduler->AddTask ([&count] () { count.fetch_add (1); }, 1ms, 1ms);

    std::this_thread::sleep_for (50ms);

    // Should execute many times
    EXPECT_GT (count.load (), 10);
    EXPECT_LT (count.load (), 100); // But not an unreasonable amount
}

// ============ Thread Safety Tests ============

TEST_F (RunningTaskSchedulerTest, GetTaskCountIsThreadSafe)
{
    std::atomic<bool> keep_adding {true};

    // Thread that adds tasks
    std::thread adder (
            [this, &keep_adding] ()
            {
                while (keep_adding.load ())
                {
                    scheduler->AddTask ([] () { std::this_thread::sleep_for (10ms); }, 5ms);
                    std::this_thread::sleep_for (5ms);
                }
            });

    // Main thread checking count
    for (int i = 0; i < 20; ++i)
    {
        size_t count = scheduler->GetTaskCount ();
        EXPECT_GE (count, 0); // Should never be negative or crash
        std::this_thread::sleep_for (10ms);
    }

    keep_adding = false;
    adder.join ();
}
