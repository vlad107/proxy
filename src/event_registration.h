#ifndef EVENT_REGISTRATION_H
#define EVENT_REGISTRATION_H
#include "epoll_handler.h"
#include <iostream>
#include <functional>

class event_registration
{
    epoll_handler *efd;
    int fd;
public:
    event_registration(epoll_handler *efd, int fd, int mask, std::function<void(int, int)> handler);
    ~event_registration();
};

#endif // EVENT_REGISTRATION_H
