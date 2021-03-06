#include "epoll_handler.h"

volatile sig_atomic_t term = 0;

epoll_handler::epoll_handler()
{
    if ((efd = epoll_create1(0)) < 0)
    {
        throw std::runtime_error("Error in epoll_create:\n" + std::string(strerror(errno)));
    }
}

epoll_handler::~epoll_handler()
{
    std::cerr << "closing epoll handler" << std::endl;
    assert(close(efd) == 0);
}

void epoll_handler::loop()
{
    struct epoll_event evs[MAX_EVENTS];
    while (!term)
    {
        int ev_sz;
        if ((ev_sz = epoll_wait(efd, evs, MAX_EVENTS, -1)) < 0)
        {
            if (EINTR == errno)
            {
                std::cerr << "epoll_wait was interrupted" << std::endl;
                continue;
            }
            throw std::runtime_error("error in epoll_wait()");
        }
        for (int i = 0; (i < ev_sz) && (!term); i++)
        {
            if (evs[i].events & (EPOLLERR | EPOLLHUP))
            {
                evs[i].events |= EPOLLRDBAND;
            }
            std::cerr << "occured on " << evs[i].data.ptr << std::endl;
            const auto& handler = reinterpret_cast<event_registration*>(evs[i].data.ptr)->get_handler();
            int fd = reinterpret_cast<event_registration*>(evs[i].data.ptr)->get_fd();
            handler(fd, evs[i].events);
        }
    }
}

void epoll_handler::add_event(int fd, int mask, event_registration *reg)
{
    epoll_event ev{};
    std::cerr << "now event_registration on " << reg << std::endl;
    ev.data.ptr = reinterpret_cast<void*>(reg);
    ev.events = mask;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) < 0)
    {
        throw std::runtime_error("error in epoll_ctl(EPOLL_CTL_ADD)\n" + std::string(strerror(errno)));
    }
}

void epoll_handler::rem_event(int fd)
{
    if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, nullptr) < 0)
    {
        throw std::runtime_error("Error in epoll_ctl(EPOLL_CTL_DEL):\n" + std::string(strerror(errno)));
    }
}

void epoll_handler::add_background_task(std::function<void()> handler)
{
    background.add_task(handler);
}
