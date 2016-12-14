#include "epoll_handler.h"

epoll_handler::epoll_handler()
{
    if ((efd = epoll_create1(0)) < 0)
    {
        throw std::runtime_error("Error in epoll_create:\n" + std::string(strerror(errno)));
    }
}

epoll_handler::~epoll_handler()
{
    assert(close(efd) == 0);
    assert(events.empty());
}

void epoll_handler::add_event(int fd, int mask, std::function<void(int)> handler)
{
    std::cerr << "try to add descriptor " << fd << " to the epoll" << std::endl;
    epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    auto data = new my_epoll_data(fd, handler);
    ev.data.ptr = reinterpret_cast<void*>(data);
    ev.events = mask;
    events[fd] = ev;
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
            if (evs[i].events & EPOLLRDHUP)
            {
                data->f(-data->get_descriptor()-1);
                rem_event(data->get_descriptor(), EPOLLIN | EPOLLRDHUP);
                continue;
            }
//            std::cerr << "descriptor " << data->get_descriptor() << " occured in epoll" << std::endl;
            data->f(data->get_descriptor());
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
    if (events.count(fd) != 0)
    {
        delete (my_epoll_data*)events[fd].data.ptr;
        events.erase(fd);
    }
    if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, &ev) < 0)
    {
        throw std::runtime_error("Error in epoll_ctl(EPOLL_CTL_DEL):\n" + std::string(strerror(errno)));
    }
    std::cerr << "descriptor removed successfully" << std::endl;
}
