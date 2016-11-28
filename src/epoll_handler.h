#ifndef EPOLL_HANDLER_H
#define EPOLL_HANDLER_H
#include <sys/epoll.h>
#include <memory.h>
#include <stdexcept>

class epoll_handler
{
    int efd;
public:
    epoll_handler();

    void add_event(int sfd, int mask);
};

#endif // EPOLL_HANDLER_H
