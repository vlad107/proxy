#include "proxy_server.h"

proxy_server::proxy_server(std::shared_ptr<epoll_handler> efd)
{
    this->efd = std::move(efd);
}

void proxy_server::start(int port)
{
    sfd = std::make_unique<server_socket>(port);
    std::cerr << "Server has been started" << std::endl;
    auto accept_handler = [this](int fd)
    {
        fd = sfd->accept(fd);
        clients[fd] = std::make_unique<transfer_data>(fd, efd);
    };
    efd->add_event(sfd->get_socket(), EPOLLIN, accept_handler);
    efd->loop();
}
