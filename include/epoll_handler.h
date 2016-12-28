#ifndef EPOLL_HANDLER_H
#define EPOLL_HANDLER_H
#include <sys/epoll.h>
#include <memory.h>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <assert.h>
#include <vector>
#include <unordered_map>
#include <unistd.h>
#include <csignal>
#include <memory>

#include "background_executor.h"
#include "event_registration.h"

extern volatile sig_atomic_t term;

class event_registration;

class epoll_handler
{
    int efd;
    std::vector<std::function<void()>> deleters;
    background_executor background;
    static const int MAX_EVENTS = 1;
public:
    epoll_handler& operator=(epoll_handler const&) = delete;
    epoll_handler(epoll_handler const&) = delete;
    epoll_handler& operator=(epoll_handler&&) = delete;
    epoll_handler(epoll_handler&&) = delete;

    epoll_handler();
    ~epoll_handler();

    void add_event(int sfd, int mask, event_registration *reg);
    void rem_event(int sfd);
    void add_deleter(std::function<void()>);
    void loop();
    void add_background_task(std::function<void()> task);
};

#endif // EPOLL_HANDLER_H
