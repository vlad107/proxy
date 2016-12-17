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
    assert(events.empty());
}

void epoll_handler::add_event(int fd, int mask, std::function<int(int, int)> handler)
{
    std::cerr << "try to add descriptor " << fd << " to the epoll" << std::endl;
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = mask;
    events[fd] = handler; // TODO
    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) < 0) // TODO: i created `ev` on stack and pass it as a parameter, ok?
    {
        throw std::runtime_error("error in epoll_ctl(EPOLL_CTL_ADD)\n" + std::string(strerror(errno)));
    }
    std::cerr << "descriptor was added" << std::endl;
}

void epoll_handler::loop()
{
    struct epoll_event evs[MAX_EVENTS];
    while (!term)
    {
        std::cerr << "============================ new iteration ============================" << std::endl;
        for (auto deleter : deleters) deleter();
        deleters.clear();
        std::cerr << "executed deleters" << std::endl;
        int ev_sz;
        if ((ev_sz = epoll_wait(efd, evs, MAX_EVENTS, -1)) < 0)
        {
            std::cerr << "error in epoll_wait" << std::endl;
            if (errno = EINTR) continue;
            throw std::runtime_error("error in epoll_wait()");
        }
        std::cerr << "executed epoll_wait" << std::endl;
        for (int i = 0; (i < ev_sz) && (!term); i++)
        {
            if (evs[i].events & (EPOLLERR | EPOLLHUP))
            {
                evs[i].events |= EPOLLRDHUP;
//                throw std::runtime_error("some error occured in epoll");
            }
            int fd = evs[i].data.fd;
            std::cerr << "event on " << fd << std::endl;
            assert(events.count(fd) != 0);
            auto cur_handler = events[fd];
            int rem = cur_handler(fd, evs[i].events);
            if (rem & EPOLLRDHUP)
            {
                std::cerr << "error in epoll" << std::endl;
            }
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

void epoll_handler::add_background_task(std::function<void()> handler)
{
    background.add_task(handler);
}

void epoll_handler::print_num_execs()
{
    background.print_num_execs();
}
