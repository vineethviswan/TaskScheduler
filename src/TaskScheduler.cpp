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
    while (running.load ())
    {
        std::unique_lock<std::mutex> lock (queue_mutex);
        if (task_queue.empty ())
        {
            cv.wait (lock, [this] { return !running.load () || !task_queue.empty (); });
            continue;
        }

        auto task = task_queue.top ();

        // Check if task is ready to execute
        if (task->next_execution > Clock::now ())
        {
            cv.wait_until (lock, task->next_execution, [this] { return !running.load (); });
            continue; // Spurious wakeup or stop requested
        }

        task_queue.pop (); // Remove task from queue
        lock.unlock (); // Unlock before executing task

        if (task->is_recurring)
        {
            // calculate next execution time
            task->next_execution += task->interval.value ();            
            task_queue.push (task);
        }
        ExecuteTask (task);
    }
}

void TaskScheduler::ExecuteTask (const std::shared_ptr<Task> &task)
{
    try
    {
        task->action ();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Task execution failed: " << e.what () << std::endl;
    }
    catch (...)
    {
        std::cerr << "Task execution failed with an unknown error." << std::endl;
    }
}

TaskID TaskScheduler::AddTask (std::function<void ()> action_, Duration delay)
{
    const TaskID taskID = next_task_id++;
    auto execution_time = Clock::now () + delay;

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
    auto task = std::make_shared<Task> (taskID, std::move (action_), execution_time, interval);
    std::lock_guard<std::mutex> lock (queue_mutex);
    task_queue.push (task);
    cv.notify_one ();

    return taskID;
}

void TaskScheduler::start ()
{
    if (running.load ())
    {
        return; // Already running
    }

    running.store (true);
    try
    {
        worker_thread = std::thread (&TaskScheduler::WorkerLoop, this);
    }
    catch (const std::exception &e)
    {
        running.store (false);
        std::cerr << e.what () << '\n';
        throw;
    }
}

void TaskScheduler::stop ()
{
    if (!running.load ())
    {
        return; // Not running
    }

    running.store (false);

    cv.notify_all (); // Wake up worker thread if waiting
    if (worker_thread.joinable ())
    {
        worker_thread.join (); // Wait for worker thread to finish
    }
}
