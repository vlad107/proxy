#ifndef EPOLL_HANDLER_H
#define EPOLL_HANDLER_H
#include <sys/epoll.h>
#include <memory.h>
#include <iostream>
#include <stdexcept>
#include <functional>

class epoll_handler
{
    int efd;
    static const int MAX_EVENTS = 1024;
public:
    epoll_handler();

    void add_event(int sfd, int mask, void (*handler)(int));
    void loop();
};

#endif // EPOLL_HANDLER_H
