#include "background_executor.h"

background_executor::background_executor()
    : alive(true)
    , num_execs()
{
    try
    {
    for (int i = 0; i < THREADS_AMOUNT; i++)
    {
        threads.push_back(std::thread([this, i]()
        {
                while (alive)
                {
                    try
                    {
                        std::unique_lock<std::mutex> lock(_mutex);
                        while (tasks.empty()) // todo: cond.wait(..., ...)
                        {
                            cond.wait(lock);
                            if (!alive)
                            {
                                return;
                            }
                        }
                        auto handler = tasks.front();
                        tasks.pop();
                        lock.unlock();
                        std::cerr << "executing at thread " << i << std::endl;
                        ++num_execs[i];
                        handler();
                    } catch (...) // WAT?
                    {
                    }
                }
        }));
    }
    } catch (...)
    {
        alive = false;
        cond.notify_all();
        for (auto &thread : threads) thread.join();
        throw;
    }
}

void background_executor::add_task(std::function<void ()> task)
{
    std::unique_lock<std::mutex> lock(_mutex);
    tasks.push(task);
    cond.notify_one();
}

background_executor::~background_executor()
{
    alive = false;
    cond.notify_all();
    for (auto &thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

void background_executor::print_num_execs()
{
    for (int i = 0; i < THREADS_AMOUNT; i++)
    {
        std::cerr << "number of executions on thread " << i << ": " << num_execs[i] << std::endl;
    }
}
