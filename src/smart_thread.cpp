#include "smart_thread.h"

smart_thread::smart_thread(std::function<void()> handler)
    : th(handler)
{
}

smart_thread::~smart_thread()
{
    if (th.joinable())
        th.join();
}
