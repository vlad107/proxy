#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H
#include "epoll_handler.h"
#include "server_socket.h"
#include "connection.h"

#include <iostream>
#include <memory>
#include <unordered_map>

extern volatile sig_atomic_t term;

class proxy_server
{
    epoll_handler *efd;
    server_socket sfd;
    event_registration reg;
    std::unordered_map<int, std::unique_ptr<connection>> conns;
public:
    proxy_server &operator=(proxy_server const&) = delete;
    proxy_server(proxy_server const&) = delete;
    proxy_server &operator=(proxy_server&&) = delete;
    proxy_server(proxy_server&&) = delete;

    proxy_server(epoll_handler *efd, int port);
    void debug()
    {
        std::cerr << "registration on " << &reg << std::endl;
    }
};

#endif // PROXY_SERVER_H
