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
        std::cerr << "============================ new iteration ============================" << std::endl;
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
//            std::cerr << "descriptor " << data->get_descriptor() << " occured in epoll" << std::endl;
            data->f();
            std::cerr << "done" << std::endl;
        }
    }
}

void epoll_handler::rem_event(int fd, int mask)
{
    std::cerr << "removing descriptor " << fd << " from epoll" << std::endl;
    epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = mask;
    if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, &ev) < 0)
    {
        throw std::runtime_error("Error in epoll_ctl(EPOLL_CTL_DEL):\n" + std::string(strerror(errno)));
    }
    std::cerr << "descriptor removed successfully" << std::endl;
}
