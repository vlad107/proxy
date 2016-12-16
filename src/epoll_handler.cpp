#include "epoll_handler.h"
// TODO: rewrite everything in event_registration

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
    assert(events.empty());
}

void epoll_handler::add_event(int fd, int mask, std::function<void(int, int)> handler)
{
    std::cerr << "try to add descriptor " << fd << " to the epoll" << std::endl;
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = mask;
    events[fd] = handler;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) < 0) // TODO: is it ok?
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
        for (auto deleter : deleters) deleter();
        deleters.clear();
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
            int fd = evs[i].data.fd;
            std::cerr << "event on " << fd << std::endl;
            assert(events.count(fd) != 0);
            auto cur_handler = events[fd];
            cur_handler(fd, evs[i].events);
        }
    }
}

void epoll_handler::rem_event(int fd)
{
    std::cerr << "removing descriptor " << fd << " from epoll" << std::endl;
    assert(events.count(fd) != 0);
    events.erase(events.find(fd));
    if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, nullptr) < 0)
    {
        throw std::runtime_error("Error in epoll_ctl(EPOLL_CTL_DEL):\n" + std::string(strerror(errno)));
    }
    std::cerr << "descriptor removed successfully" << std::endl;
}

void epoll_handler::add_deleter(std::function<void ()> func)
{
    deleters.push_back(func);
}
