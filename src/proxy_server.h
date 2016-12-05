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
    std::map<int, std::unique_ptr<transfer_data>> clients;
public:
    proxy_server(epoll_handler *efd, int port);
    ~proxy_server();
    void start(int port);
};

#endif // PROXY_SERVER_H
