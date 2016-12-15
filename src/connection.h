#ifndef CONNECTION_H
#define CONNECTION_H
#include "epoll_handler.h"
#include "transfer_data.h"
#include "event_registration.h"

class connection
{
    std::function<void()> disconnect_handler;
    std::unique_ptr<transfer_data> data;
    int client_fd;
    epoll_handler *efd;
    std::unique_ptr<event_registration> reg;
public:
    connection(int client_fd, epoll_handler *efd);
    void set_disconnect(std::function<void()> disconnect_handler);
    void start();
};

#endif // CONNECTION_H
