#include "event_registration.h"

event_registration::event_registration(epoll_handler *efd, int fd, int mask, std::function<int(int, int)> handler)
    : efd(efd),
      fd(fd)
{

    efd->add_event(fd, mask, handler);
}

event_registration::event_registration(event_registration &&other)
{
    efd = other.efd;
    fd = other.fd;
}

event_registration::event_registration()
{
    fd = -1;
}

event_registration::~event_registration()
{
    if (fd != -1)
    {
        efd->rem_event(fd);
    }
}

event_registration& event_registration::operator=(event_registration &&other)
{
    std::swap(efd, other.efd);
    std::swap(fd, other.fd);
    return *this;
}

