#ifndef SMART_THREAD_H
#define SMART_THREAD_H
#include <thread>

class smart_thread
{
    std::thread th;
public:
    smart_thread(std::function<void()> handler);
    smart_thread(smart_thread&&) = default;
    smart_thread& operator=(smart_thread&&) = default;
    ~smart_thread();
};

#endif // SMART_THREAD_H
