#include "connection.h"

connection::connection(sockfd cfd, epoll_handler *efd)
    : efd(efd),
      _was_disconnect_handler(false),
      data(std::move(cfd), efd)
{
}

void connection::set_disconnect(std::function<void ()> disconnect_handler)
{
    _was_disconnect_handler = true;
    this->disconnect_handler = disconnect_handler;
    data.set_disconnect(disconnect_handler);        // TODO: how to say client that it should be closed?
}

void connection::start()
{
    assert(_was_disconnect_handler);
    reg = std::move(event_registration(efd,
                                       data.get_client_infd(),
                                       EPOLLIN | EPOLLRDHUP,
                                       [this](int _fd, int _event)
    { // first should be EPOLLIN
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
