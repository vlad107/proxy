#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H
#include "epoll_handler.h"
#include "server_socket.h"
#include "transfer_data.h"

#include <iostream>
#include <memory>

class proxy_server
{
    epoll_handler *efd;
    std::unique_ptr<server_socket> sfd;
public:
    proxy_server &operator=(proxy_server const&) = delete;
    proxy_server(proxy_server const&) = delete;
    proxy_server &operator=(proxy_server&&) = delete;
    proxy_server(proxy_server&&) = delete;

    proxy_server(epoll_handler *efd, int port);
    ~proxy_server();
    void start(int port);
};

#endif // PROXY_SERVER_H
