//
// Created by Vineeth on 2025-07-01.
//

#ifndef TASK_H
#define TASK_H
#include <atomic>
#include <chrono>
#include <functional>

using Action = std::function<void()>;
using TaskID = std::size_t;
using Duration = std::chrono::milliseconds;

class TaskScheduler {
public:
    TaskScheduler() = default;
    ~TaskScheduler() = default;

    // Add delayed task (executes once after delay)
    TaskID AddTask(std::function<void()> action, Duration delay);

    // Add recurring task (executes repeatedly at interval)
    TaskID AddTask(std::function<void()> action_, Duration delay, Duration interval);

    void start();
    void stop();
    bool isRunning() const { return running.load(); }

private:
    Action action;
    std::atomic<bool> running = false;
    std::atomic<TaskID> nextTaskID = 0;
};

#endif // TASK_H
