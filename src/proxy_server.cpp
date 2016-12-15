#include "proxy_server.h"

proxy_server::proxy_server(epoll_handler *efd, int port)
{
    this->efd = efd;
    sfd = std::make_unique<server_socket>(port);
    std::cerr << "Server has been started" << std::endl;

    auto accept_handler = [this, efd](int fd, int event)
    {
        if (event & EPOLLIN)
        {
            int cfd = sfd->accept(fd);
            auto conn = std::make_unique<connection>(cfd, efd);
            auto ptr = conns.insert(std::move(conn)).first;
            (*ptr)->set_disconnect([this, ptr]()
            {
                conns.erase(ptr);
            });
            (*ptr)->start();
            event ^= EPOLLIN;
        }
        if (event & EPOLLRDHUP)
        {
            std::cerr << "server on " << fd << " disconnected" << std::endl;
            std::cerr << "so, what should I do here?" << std::endl;
            // TODO: something else should be here
            event ^= EPOLLRDHUP;
        }
        assert(event == 0);
    };
    reg = std::make_unique<event_registration>(efd,
                                               sfd->get_socket(),
                                               EPOLLIN | EPOLLRDHUP,
                                               accept_handler);
}
