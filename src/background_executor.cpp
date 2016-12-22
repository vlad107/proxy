#include "background_executor.h"

background_executor::background_executor()
    : alive(true)
{
    // TODO: will threads.resize(THREADS_AMOUNT) work?
    for (size_t i = 0; i < THREADS_AMOUNT; i++)
    {
        threads.push_back(std::make_unique<smart_thread>(&alive, [this, i]()
        {
                while (alive)
                {
                    try
                    {
                        std::function<void()> handler;
                        {
                            std::unique_lock<std::mutex> lock(_mutex);
                            cond.wait(lock, [this]()
                            {
                                return !tasks.empty() || !alive;
                            });
                            if (!alive) return;
                            handler = tasks.front();
                            tasks.pop();
                        }
                        handler();
                    } catch (const std::exception &e)
                    {
                        std::cerr << e.what() << std::endl;
                    }
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

background_executor::~background_executor()
{
    alive = false;
    cond.notify_all();
}
