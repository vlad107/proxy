#include "event_registration.h"

event_registration::event_registration(epoll_handler *efd, int fd, int mask, std::function<int(int, int)> handler)
{
    this->efd = efd;
    this->fd = fd;
    efd->add_event(fd, mask, handler);
}

event_registration::~event_registration()
{
    efd->rem_event(fd);
}
