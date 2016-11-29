#include "epoll_handler.h"

epoll_handler::epoll_handler()
{
    efd = epoll_create1(0);
}

void epoll_handler::add_event(int fd, int mask, std::function<void(int)> handler)
{
    std::cerr << "try to add descriptor " << fd << " to the epoll" << std::endl;
    epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    my_epoll_data* data = new my_epoll_data(fd, handler);
    ev.data.ptr = reinterpret_cast<void*>(data);
    ev.events = mask;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) < 0)
    {
        throw std::runtime_error("error in epoll_ctl(EPOLL_CTL_ADD)\n" + std::string(strerror(errno)));
    }
    std::cerr << "descriptor was added" << std::endl;
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
            auto data = reinterpret_cast<my_epoll_data*>(evs[i].data.ptr);
            std::cerr << "descriptor " << evs[i].data.fd << " occured in epoll" << std::endl;
            data->f();
        }
    }
}
