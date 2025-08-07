//
// Created by Vineeth on 2025-07-01.
//

#ifndef TASK_H
#define TASK_H
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

#include "Logger.h"

using Action = std::function<void ()>;
using TaskID = std::size_t;
using Duration = std::chrono::milliseconds;
using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

class TaskScheduler
{
private:
    static constexpr auto DEFAULT_SHUTDOWN_TIMEOUT = std::chrono::seconds {15};

    struct Task
    {
        TaskID id;
        Action action;
        TimePoint next_execution;
        std::optional<Duration> interval; // If empty, it's a one-time task
        bool is_recurring;

        Task (TaskID id_, Action action_, TimePoint next_execution_, std::optional<Duration> interval_ = std::nullopt) :
            id (id_), action (std::move (action_)), next_execution (next_execution_), interval (interval_),
            is_recurring (interval_.has_value ())
        {
            auto delay = std::chrono::duration_cast<std::chrono::milliseconds> (next_execution - Clock::now ());
            Logger::Log (Logger::Level::INFO, "Task {} created, executing in {} ms", id, delay.count ());
        }
    };

    struct TaskComparator
    {
        bool operator() (const std::shared_ptr<Task> &lhs, const std::shared_ptr<Task> &rhs) const
        {
            return lhs->next_execution > rhs->next_execution; // Min-heap based on next execution time
        }
    };

    void WorkerLoop ();
    void ExecuteTask (const std::shared_ptr<Task> &task);

    std::priority_queue<std::shared_ptr<Task>, std::vector<std::shared_ptr<Task>>, TaskComparator> task_queue;
    std::atomic<bool> running {false};
    std::atomic<TaskID> next_task_id {0};
    std::mutex queue_mutex;
    std::condition_variable cv;
    std::thread worker_thread;

    std::atomic<bool> stopping {false}; // Signal for graceful shutdown
    std::atomic<size_t> active_tasks {0}; // Count of currently executing tasks
    std::condition_variable shutdown_cv; // Condition variable for shutdown synchronization

public:
    TaskScheduler () = default;
    ~TaskScheduler ()
    {
        if (active_tasks.load () == 0)
            Stop ();
    }

    enum class ShutdownMode
    {
        IMMEDIATE, // Stop all tasks immediately
        DRAIN_QUEUE, // Wait for all tasks in the queue to finish
        COMPLETE_CURRENT // Wait for currently executing tasks to finish
    };

    // Non-copyable and non-movable
    TaskScheduler (const TaskScheduler &) = delete;
    TaskScheduler (TaskScheduler &&) = delete;
    TaskScheduler &operator= (const TaskScheduler &) = delete;

    // Add delayed task (executes once after delay)
    TaskID AddTask (std::function<void ()> action, Duration delay = Duration::zero ());

    // Add recurring task (executes repeatedly at interval)
    TaskID AddTask (std::function<void ()> action_, Duration delay, Duration interval);

    void Start ();
    void Stop (ShutdownMode mode = ShutdownMode::DRAIN_QUEUE);
    bool IsRunning () const { return running.load (); }
    size_t GetTaskCount ();
};

#endif // TASK_H
