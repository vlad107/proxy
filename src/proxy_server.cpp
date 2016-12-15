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
            auto data = std::make_shared<transfer_data>(cfd, efd);
            efd->add_event(cfd, EPOLLIN | EPOLLRDHUP, [data, efd](int _fd, int _event)
            {
                if (_event & EPOLLIN)
                {
                    std::cerr << "EPOLLIN on " << _fd << " client" << std::endl;
                    data->data_occured(_fd);
                    _event ^= EPOLLIN;
                }
                if (_event & EPOLLRDHUP)
                {
                    std::cerr << "EPOLLRDHUP on " << _fd << " client" << std::endl;
                    std::cerr << "i'm here" << std::endl;
                    efd->rem_event(_fd, EPOLLIN | EPOLLRDHUP);
                    _event ^= EPOLLRDHUP;
                }
                assert(_event == 0);
            });
            event ^= EPOLLIN;
        }
        if (event & EPOLLRDHUP)
        {
            std::cerr << "server on " << fd << " disconnected" << std::endl;
            // TODO: something else should be here
            event ^= EPOLLRDHUP;
        }
        assert(event == 0);
    };
    efd->add_event(sfd->get_socket(), EPOLLIN, accept_handler);
}

proxy_server::~proxy_server()
{
    efd->rem_event(sfd->get_socket(), EPOLLIN);
}
