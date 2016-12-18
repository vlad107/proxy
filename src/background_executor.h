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

class background_executor
{
    static const size_t THREADS_AMOUNT = 4;
    std::vector<std::unique_ptr<smart_thread>> threads;
    std::mutex _mutex;
    std::queue<std::function<void()>> tasks;
    std::condition_variable cond;
    bool alive;
public:
    background_executor();
    ~background_executor();
    void add_task(std::function<void()> task);
};

#endif // BACKGROUND_EXECUTOR_H
