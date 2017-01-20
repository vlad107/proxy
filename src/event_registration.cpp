#include "event_registration.h"

event_registration::event_registration(epoll_handler *efd, int fd, int mask, std::function<int(int, int)> handler)
    : efd(efd),
      fd(fd),
      handler(handler),
      mask(mask)
{
    efd->add_event(fd, mask, this);
    std::cerr << "event registration created in " << this << std::endl;
}

event_registration::event_registration(event_registration &&other)
{   // TODO: the same code in operator =
    efd = other.efd;
    fd = other.fd;
    handler = other.handler;
    mask = other.mask;
    efd->rem_event(other.fd);
    efd->add_event(fd, mask, this);
    other.clean();
}

event_registration::event_registration()
    : efd(nullptr),
      fd(-1)
{
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
    efd = other.efd;
    fd = other.fd;
    handler = other.handler;
    mask = other.mask;
    efd->rem_event(fd);
    efd->add_event(fd, mask, this);
    other.clean();
    return *this;
}

std::function<void(int, int)> event_registration::get_handler()
{
    return handler;
}

int event_registration::get_fd()
{
    return fd;
}

void event_registration::clean()
{
    efd = nullptr;
    fd = -1;
    handler = nullptr;
    mask = 0;
}
