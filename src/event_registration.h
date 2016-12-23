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
    event_registration& operator=(const event_registration &)= delete;
    event_registration(const event_registration &) = delete;
    event_registration(event_registration&&);
    event_registration& operator=(event_registration&&);

    event_registration();
    event_registration(epoll_handler *efd, int fd, int mask, std::function<int(int, int)> handler);
    ~event_registration();
};

#endif // EVENT_REGISTRATION_H
