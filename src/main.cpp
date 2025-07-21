
#include <iostream>
#include "TaskScheduler.h"

int main()
{
    std::cout << "Task Scheduler\n";

    TaskScheduler tsh;
    tsh.AddTask([](){ std::cout << "Task 1\n";}, std::chrono::milliseconds(1000));
    tsh.start();
}
