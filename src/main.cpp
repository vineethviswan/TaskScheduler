
#include "TaskScheduler.h"

int main ()
{
    std::cout << "Task Scheduler\n";
    TaskScheduler scheduler;

    scheduler.AddTask([] { std::cout << "One-time task 1 executed immediately.\n"; });

    scheduler.AddTask ([] { std::cout << "One-time task 2 executed after 1 second.\n"; }, std::chrono::seconds (1));

    scheduler.AddTask ([] { std::cout << "Recurring task 3 executed every 2 seconds.\n"; }, std::chrono::seconds (2),
            std::chrono::seconds (2));

    scheduler.Start ();

    // Keep main thread alive
    std::cout << "Press Enter to stop...\n";
    std::cin.get ();

    //scheduler.Stop (TaskScheduler::ShutdownMode::DRAIN_QUEUE);
    //scheduler.Stop (TaskScheduler::ShutdownMode::IMMEDIATE);
    scheduler.Stop (TaskScheduler::ShutdownMode::COMPLETE_CURRENT);

    return 0;
}
