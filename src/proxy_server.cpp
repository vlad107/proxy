#include "proxy_server.h"

proxy_server::proxy_server(epoll_handler *efd, int port)
{
    this->efd = efd;
    sfd = std::make_unique<server_socket>(port);
    std::cerr << "Server has been started" << std::endl;
    auto accept_handler = [this, efd](int fd)
    {
        if (fd < 0)
        {
            fd = -fd - 1;
            assert(clients.count(fd) > 0);
            std::cerr << "removing " << fd << " descriptor" << std::endl;
            clients.erase(fd);
        } else
        {
            std::cerr << "accept " << fd << " descriptor" << std::endl;
            int cfd = sfd->accept(fd);
            clients[cfd] = std::make_unique<transfer_data>(cfd, efd); // TODO: map is unnecessary
        }
    };
    efd->add_event(sfd->get_socket(), EPOLLIN, accept_handler);
}

proxy_server::~proxy_server()
{
    efd->rem_event(sfd->get_socket(), EPOLLIN);
}
