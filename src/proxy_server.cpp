#include "proxy_server.h"

proxy_server::proxy_server(std::unique_ptr<epoll_handler> efd)
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
        clients[fd] = std::make_unique<client_data>(fd);
        clients[fd]->make_nonblocking();
        auto client_handler = [&](int ffd)
        {
            std::cerr << "client with descriptor " << ffd << " is available for reading" << std::endl;
            clients[ffd]->read_all();
            clients[ffd]->check_for_requests();
        };

        efd->add_event(clients[fd]->get_descriptor(), EPOLLIN, client_handler);

    };
    efd->add_event(sfd->get_socket(), EPOLLIN, accept_handler);
    efd->loop();
}
