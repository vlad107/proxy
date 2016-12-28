#ifndef EVENT_REGISTRATION_H
#define EVENT_REGISTRATION_H
#include "epoll_handler.h"
#include <iostream>
#include <functional>

class epoll_handler;

class event_registration
{
    epoll_handler *efd;
    int fd;
    std::function<void(int, int)> handler;
    int mask;

    void clean();
public:
    event_registration& operator=(const event_registration &)= delete;
    event_registration(const event_registration &) = delete;

    event_registration(event_registration&&);
    event_registration& operator=(event_registration&&);

    event_registration();
    event_registration(epoll_handler *efd, int fd, int mask, std::function<int(int, int)> handler);
    ~event_registration();

    void execute(int);
    void debug()
    {
//        std::cerr << fd << " " << efd << std::endl;
    }
};

#endif // EVENT_REGISTRATION_H
