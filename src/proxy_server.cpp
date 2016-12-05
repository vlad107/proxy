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
            if (clients.count(fd) != 0)
            {
                std::cerr << "removing " << fd << " descriptor" << std::endl;
                clients.erase(fd);
            }
        }
        std::cerr << "accept " << fd << " descriptor" << std::endl;
        fd = sfd->accept(fd);
        clients[fd] = std::make_unique<transfer_data>(fd, efd);
    };
    efd->add_event(sfd->get_socket(), EPOLLIN, accept_handler);
}

proxy_server::~proxy_server()
{
    efd->rem_event(sfd->get_socket(), EPOLLIN);
}
