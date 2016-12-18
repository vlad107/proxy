#include "smart_thread.h"

smart_thread::smart_thread(bool *alive, std::function<void()> handler)
    : alive(alive), th(handler)
{
}

smart_thread::~smart_thread()
{
    *alive = false;
    join();
}

void smart_thread::join()
{
    th.join();
}
