#include "background_executor.h"

background_executor::background_executor()
{
    for (int i = 0; i < THREADS_AMOUNT; i++)
    {
        threads.push_back(std::thread([this, i]()
        {
            while (1)
            {
                std::unique_lock<std::mutex> lock(_mutex);
                while (tasks.empty())
                {
                    cond.wait(lock);
                }
                auto handler = tasks.front();
                tasks.pop();
                lock.unlock();
                std::cerr << "executing at thread " << i << std::endl;
                handler();
            }
        }));
    }
}

void background_executor::add_task(std::function<void ()> task)
{
    std::unique_lock<std::mutex> lock(_mutex);
    tasks.push(task);
    cond.notify_one();
}
