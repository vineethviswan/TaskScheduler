//
// Created by Vineeth on 2025-07-06.
//

#include "TaskScheduler.h"

TaskID TaskScheduler::AddTask(std::function<void()> action_, Duration delay) {
    const TaskID taskID = nextTaskID++;
    action = action_;
    return taskID;
}

TaskID TaskScheduler::AddTask(std::function<void()> action_, Duration delay, Duration interval) {
    const TaskID taskID = nextTaskID++;
    action = action_;
    return taskID;
}

void TaskScheduler::start() { }
void TaskScheduler::stop() { }
