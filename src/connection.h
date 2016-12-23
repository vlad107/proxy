#ifndef CONNECTION_H
#define CONNECTION_H
#include "epoll_handler.h"
#include "transfer_data.h"
#include "sockfd.h"
#include "event_registration.h"


class connection
{
    epoll_handler *efd;
    bool _was_disconnect_handler;
    transfer_data data;
    std::function<void()> disconnect_handler;
    event_registration reg;
public:
    connection(connection const &) = delete;
    connection(connection&&) = delete;
    connection &operator =(connection const &) = delete;
    connection &operator =(connection &&) = delete;

    connection(sockfd cfd, epoll_handler *efd);
    void set_disconnect(std::function<void()> disconnect_handler);
    void start();
};

#endif // CONNECTION_H
