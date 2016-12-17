#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H
#include "epoll_handler.h"
#include "server_socket.h"
#include "connection.h"

#include <iostream>
#include <memory>
#include <set>

extern volatile sig_atomic_t term;

class proxy_server
{
    epoll_handler *efd;
    std::unique_ptr<server_socket> sfd;
    std::set<std::unique_ptr<connection>> conns;
    std::unique_ptr<event_registration> reg;
public:
    proxy_server &operator=(proxy_server const&) = delete;
    proxy_server(proxy_server const&) = delete;
    proxy_server &operator=(proxy_server&&) = delete;
    proxy_server(proxy_server&&) = delete;

    proxy_server(epoll_handler *efd, int port);
};

#endif // PROXY_SERVER_H
