#ifndef EPOLL_HANDLER_H
#define EPOLL_HANDLER_H
#include <sys/epoll.h>
#include <memory.h>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <assert.h>
#include <unordered_map>
#include <unistd.h>
#include <memory>

class my_epoll_data
{
    int fd;
    std::function<void(int)> func;
public:
    my_epoll_data(int fd, std::function<void(int)> func) : fd(fd), func(func) {}
    void f(int fd)
    {
        func(fd);
    }
    int get_descriptor()
    {
        return fd;
    }
};

class epoll_handler
{
    int efd;
    std::unordered_map<int, epoll_event> events;
    static const int MAX_EVENTS = 1024;
public:
    epoll_handler& operator=(epoll_handler const&) = delete;
    epoll_handler(epoll_handler const&) = delete;
    epoll_handler& operator=(epoll_handler&&) = delete;
    epoll_handler(epoll_handler&&) = delete;

    epoll_handler();
    ~epoll_handler();

    void add_event(int sfd, int mask, std::function<void(int)> handler);
    void rem_event(int sfd, int mask);
    void loop();
};

#endif // EPOLL_HANDLER_H
