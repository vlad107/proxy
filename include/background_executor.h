#ifndef BACKGROUND_EXECUTOR_H
#define BACKGROUND_EXECUTOR_H
#include <pthread.h>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>
#include <queue>
#include <condition_variable>
#include <memory.h>

#include "smart_thread.h"

class notifier
{
public:
    notifier(bool& alive, std::condition_variable& cond);

    notifier(notifier const&) = delete;
    notifier& operator=(notifier const&) = delete;

    ~notifier();

private:
    bool& alive;
    std::condition_variable& cond;
};

class background_executor
{
    static const size_t THREADS_AMOUNT = 16;
    std::mutex _mutex;
    std::queue<std::function<void()>> tasks;
    std::condition_variable cond;
    bool alive;
    std::vector<smart_thread> threads;
    notifier notify_threads;
public:
    background_executor();
    void add_task(std::function<void()> task);
};

#endif // BACKGROUND_EXECUTOR_H
