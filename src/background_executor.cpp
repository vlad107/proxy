#include "background_executor.h"

notifier::notifier(bool& alive, std::condition_variable& cond)
    : alive(alive)
    , cond(cond)
{}

notifier::~notifier()
{
    alive = false;
    cond.notify_all();
}

background_executor::background_executor()
    : alive(true)
    , notify_threads(alive, cond)
{
    // TODO: will threads.resize(THREADS_AMOUNT) work?
    for (size_t i = 0; i < THREADS_AMOUNT; i++)
    {
        threads.emplace_back([this, i]()
        {
            while (alive)
            {
                try
                {
                    std::cerr << "THREAD " << i << " IS AVAILABLE NOW" << std::endl;
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
        });
    }
}

void background_executor::add_task(std::function<void ()> task)
{
    std::unique_lock<std::mutex> lock(_mutex);
    tasks.push(task);
    cond.notify_one();
}
