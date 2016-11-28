#include "epoll_handler.h"

epoll_handler::epoll_handler()
{
    efd = epoll_create1(0);
}

void epoll_handler::add_event(int fd, int mask)
{
    epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.fd = fd;
    ev.events = mask;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) < 0)
    {
        throw std::runtime_error("error in epoll_ctl(EPOLL_CTL_ADD)");
    }
}
