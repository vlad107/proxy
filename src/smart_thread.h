#ifndef SMART_THREAD_H
#define SMART_THREAD_H
#include <thread>

class smart_thread
{
    std::thread th;
public:
    smart_thread(std::function<void()> handler);
    ~smart_thread();
    void join();
};

#endif // SMART_THREAD_H
