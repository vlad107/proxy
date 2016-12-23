#include "connection.h"

connection::connection(sockfd cfd, epoll_handler *efd)
    : efd(efd),
      _was_disconnect_handler(false),
      data(std::move(cfd), efd),
      reg()
{
}

void connection::set_disconnect(std::function<void ()> disconnect_handler)
{
    _was_disconnect_handler = true;
    this->disconnect_handler = std::move(disconnect_handler);
}

void connection::start()
{
    assert(_was_disconnect_handler);
    reg = std::move(event_registration(efd,
                                       data.get_client_infd(),
                                       EPOLLIN | EPOLLRDHUP,
                                       [this](int _fd, int _event)
    {
        if (_event & EPOLLIN)
        {
            data.data_occured(_fd);
            _event ^= EPOLLIN;
        }
        if (_event & EPOLLRDHUP)
        {
            disconnect_handler();
            _event ^= EPOLLRDHUP;
        }
        return _event;
    }));
}
