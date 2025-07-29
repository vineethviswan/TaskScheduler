//
// Created by Vineeth on 2025-07-06.
//

#include "TaskScheduler.h"

size_t TaskScheduler::GetTaskCount ()
{
    std::lock_guard<std::mutex> lock (queue_mutex);
    return task_queue.size ();
}

void TaskScheduler::WorkerLoop ()
{
    Logger::Log (Logger::Level::INFO, "Worker thread started.");
    while (running.load ())
    {
        std::unique_lock<std::mutex> lock (queue_mutex);
        if (task_queue.empty ())
        {
            Logger::Log (Logger::Level::DEBUG, "Queue empty, waiting for tasks");
            cv.wait (lock, [this] { return !running.load () || !task_queue.empty (); });
            continue;
        }

        auto task = task_queue.top ();
        auto now = Clock::now ();

        // Check if task is ready to execute
        if (task->next_execution > now)
        {
            auto delay = std::chrono::duration_cast<std::chrono::milliseconds> (task->next_execution - now);
            Logger::Log (Logger::Level::DEBUG, "Task {} scheduled in {} ms", task->id, delay.count ());
            cv.wait_until (lock, task->next_execution, [this] { return !running.load (); });
            continue; // Spurious wakeup or stop requested
        }

        task_queue.pop (); // Remove task from queue
        lock.unlock (); // Unlock before executing task

        if (task->is_recurring)
        {
            // calculate next execution time
            task->next_execution += task->interval.value ();
            Logger::Log (Logger::Level::DEBUG, "Rescheduling recurring task {} for next execution", task->id);
            task_queue.push (task);
        }
        Logger::Log (Logger::Level::INFO, "Executing task {}", task->id);
        ExecuteTask (task);
    }
    Logger::Log (Logger::Level::INFO, "Worker thread stopped");
}

void TaskScheduler::ExecuteTask (const std::shared_ptr<Task> &task)
{
    try
    {
        task->action ();
        Logger::Log (Logger::Level::INFO, "Task {} completed successfully", task->id);
    }
    catch (const std::exception &e)
    {
        Logger::Log (Logger::Level::ERROR, "Task {} failed: {}", task->id, e.what ());
    }
    catch (...)
    {
        Logger::Log (Logger::Level::ERROR, "Task {} failed with unknown error", task->id);
    }
}

TaskID TaskScheduler::AddTask (std::function<void ()> action_, Duration delay)
{
    const TaskID taskID = next_task_id++;
    auto execution_time = Clock::now () + delay;

    Logger::Log (Logger::Level::INFO, "Adding one-time task {} with delay {} ms", taskID,
            std::chrono::duration_cast<std::chrono::milliseconds> (delay).count ());

    auto task = std::make_shared<Task> (taskID, std::move (action_), execution_time);
    std::lock_guard<std::mutex> lock (queue_mutex);
    task_queue.push (task);
    cv.notify_one ();

    return taskID;
}

TaskID TaskScheduler::AddTask (std::function<void ()> action_, Duration delay, Duration interval)
{
    const TaskID taskID = next_task_id++;
    auto execution_time = Clock::now () + delay;

    Logger::Log (Logger::Level::INFO, "Adding recurring task {} with initial delay {} ms and interval {} ms", taskID,
            std::chrono::duration_cast<std::chrono::milliseconds> (delay).count (),
            std::chrono::duration_cast<std::chrono::milliseconds> (interval).count ());

    auto task = std::make_shared<Task> (taskID, std::move (action_), execution_time, interval);
    std::lock_guard<std::mutex> lock (queue_mutex);
    task_queue.push (task);
    cv.notify_one ();

    return taskID;
}

void TaskScheduler::Start ()
{
    if (running.load ())
    {
        Logger::Log (Logger::Level::WARNING, "Attempted to start an already running scheduler");
        return; // Already running
    }

    Logger::Log (Logger::Level::INFO, "Starting task scheduler");
    running.store (true);
    try
    {
        worker_thread = std::thread (&TaskScheduler::WorkerLoop, this);
    }
    catch (const std::exception &e)
    {
        running.store (false);
        Logger::Log (Logger::Level::ERROR, "Failed to start scheduler: {}", e.what ());
        throw;
    }
}

void TaskScheduler::Stop ()
{
    if (!IsRunning ())
    {
        Logger::Log (Logger::Level::WARNING, "Attempted to stop a non-running scheduler");
        return; // Not running
    }

    Logger::Log (Logger::Level::INFO, "Stopping task scheduler");
    running.store (false);

    cv.notify_all (); // Wake up worker thread if waiting
    if (worker_thread.joinable ())
    {
        worker_thread.join (); // Wait for worker thread to finish
        Logger::Log (Logger::Level::INFO, "Task scheduler stopped successfully");
    }
}
