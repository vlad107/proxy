#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H
#include "epoll_handler.h"
#include "server_socket.h"
#include <iostream>
#include <memory>

class proxy_server
{
    std::unique_ptr<epoll_handler> efd;
    std::unique_ptr<server_socket> sfd;
public:
    proxy_server(std::unique_ptr<epoll_handler> efd);
    void start(int port);
};

#endif // PROXY_SERVER_H
