#include "proxy_server.h"

proxy_server::proxy_server(std::unique_ptr<epoll_handler> efd)
{
    this->efd = std::move(efd);
}

void proxy_server::start(int port)
{
    sfd = std::make_unique<server_socket>(port);
    efd->add_event(sfd->get_socket(), EPOLLIN);
}
