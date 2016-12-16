#include "connection.h"

connection::connection(int client_fd, epoll_handler *efd) : client_fd(client_fd), efd(efd)
{
    data = std::make_unique<transfer_data>(client_fd, efd);
}

void connection::set_disconnect(std::function<void ()> disconnect_handler)
{
    this->disconnect_handler = disconnect_handler;
}

void connection::start()
{
    reg = std::make_unique<event_registration>(efd,
                                               client_fd,
                                               EPOLLIN | EPOLLRDHUP,
                                               [this](int _fd, int _event)
    {
        if (_event & EPOLLIN)
        {
            data->data_occured(_fd);
            _event ^= EPOLLIN;
        }
        if (_event & EPOLLRDHUP)
        {
            efd->add_deleter(disconnect_handler);
            _event ^= EPOLLRDHUP;
        }
        assert(_event == 0);
    });
}
