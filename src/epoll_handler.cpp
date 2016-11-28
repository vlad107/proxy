#include "epoll_handler.h"

epoll_handler::epoll_handler()
{
    efd = epoll_create1(0);
}

void epoll_handler::add_event(int fd, int mask, void (*handler) (int))
{
    epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.fd = fd;
    ev.data.ptr = reinterpret_cast<void*>(handler);
    ev.events = mask;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) < 0)
    {
        throw std::runtime_error("error in epoll_ctl(EPOLL_CTL_ADD)");
    }
}

void epoll_handler::loop()
{
    struct epoll_event evs[MAX_EVENTS];
    while (true)
    {
        int ev_sz;
        if ((ev_sz = epoll_wait(efd, evs, MAX_EVENTS, -1)) < 0)
        {
            if (errno = EINTR) continue;
            throw std::runtime_error("error in epoll_wait()");
        }
        for (int i = 0; i < ev_sz; i++)
        {
            if (evs[i].events & (EPOLLERR | EPOLLHUP))
            {
                throw std::runtime_error("some error occured in epoll");
            }
            auto f = reinterpret_cast<void(*)(int)>(evs[i].data.ptr);
            f(evs[i].data.fd);
        }
    }
}
