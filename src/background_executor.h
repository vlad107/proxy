#ifndef BACKGROUND_EXECUTOR_H
#define BACKGROUND_EXECUTOR_H
#include <pthread.h>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>
#include <queue>
#include <condition_variable>

class background_executor
{
    std::vector<std::thread> threads;
    std::mutex _mutex;
    std::queue<std::function<void()>> tasks;
    std::condition_variable cond;
    static const int THREADS_AMOUNT = 4;
public:
    background_executor();
    void add_task(std::function<void()> task);
};

#endif // BACKGROUND_EXECUTOR_H
