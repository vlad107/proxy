#ifndef SMART_THREAD_H
#define SMART_THREAD_H
#include <thread>

class smart_thread
{
    bool *alive;
    std::thread th;
public:
    smart_thread(bool *alive, std::function<void()> handler);
    ~smart_thread();
    void join();
};

#endif // SMART_THREAD_H
