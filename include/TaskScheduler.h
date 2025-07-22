//
// Created by Vineeth on 2025-07-01.
//

#ifndef TASK_H
#define TASK_H
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <queue>
#include <thread>
#include <condition_variable>
#include <optional>
#include <vector>
#include <mutex>
#include <iostream>

using Action = std::function<void ()>;
using TaskID = std::size_t;
using Duration = std::chrono::milliseconds;
using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

class TaskScheduler
{
private:
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
        }
    };

    struct TaskComparator
    {
        bool operator() (const std::shared_ptr<Task> &lhs, const std::shared_ptr<Task> &rhs) const
        {
            return lhs->next_execution > rhs->next_execution; // Min-heap based on next execution time
        }
    };

    void WorkerLoop();
    void ExecuteTask (const std::shared_ptr<Task> &task);

    std::priority_queue<std::shared_ptr<Task>, std::vector<std::shared_ptr<Task>>, TaskComparator> task_queue;
    std::atomic<bool> running {false};
    std::atomic<TaskID> next_task_id {0};
    std::mutex queue_mutex;
    std::condition_variable cv;
    std::thread worker_thread;

public:
    TaskScheduler () = default;
    ~TaskScheduler () {
        stop ();
    }

    // Non-copyable and non-movable
    TaskScheduler (const TaskScheduler &) = delete;
    TaskScheduler (TaskScheduler &&) = delete;
    TaskScheduler &operator= (const TaskScheduler &) = delete;

    // Add delayed task (executes once after delay)
    TaskID AddTask (std::function<void ()> action, Duration delay);

    // Add recurring task (executes repeatedly at interval)
    TaskID AddTask (std::function<void ()> action_, Duration delay, Duration interval);

    void start ();
    void stop ();
    bool isRunning () const { return running.load (); }
    size_t GetTaskCount ();
};

#endif // TASK_H
