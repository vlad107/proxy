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

class background_executor
{
    static const int THREADS_AMOUNT = 4;
    std::vector<std::thread> threads;
    std::mutex _mutex;
    std::queue<std::function<void()>> tasks;
    std::condition_variable cond;
    bool alive;
    int num_execs[THREADS_AMOUNT];
public:
    background_executor();
    ~background_executor();
    void print_num_execs();
    void add_task(std::function<void()> task);
};

#endif // BACKGROUND_EXECUTOR_H
