
#include "TaskScheduler.h"

int main ()
{
    std::cout << "Task Scheduler\n";
    TaskScheduler scheduler;

    scheduler.AddTask([] { std::cout << "One-time task executed immediately.\n"; });

    scheduler.AddTask ([] { std::cout << "One-time task executed after 1 second.\n"; }, std::chrono::seconds (1));

    scheduler.AddTask ([] { std::cout << "Recurring task executed every 2 seconds.\n"; }, std::chrono::seconds (2),
            std::chrono::seconds (2));

    scheduler.Start ();

    // Keep main thread alive
    std::cout << "Press Enter to stop...\n";
    std::cin.get ();

    scheduler.Stop ();

    return 0;
}
