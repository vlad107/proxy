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
            efd->add_deleter(disconnect_handler);
            _event ^= EPOLLRDHUP;
        }
        return _event;
    }));
}

//connection::connection(connection &&other)
//    : efd(other.efd),
//      _was_disconnect_handler(other._was_disconnect_handler),
//      data(-1, 0),  // TODO
//      reg(),        // TODO
//      disconnect_handler(other.disconnect_handler)
//{
//}

//connection &connection::operator =(connection &&other)
//{
//    std::swap(efd, other.efd);
//    std::swap(_was_disconnect_handler, other._was_disconnect_handler);
////    TODO: data
////    TODO: reg
//    std::swap(disconnect_handler, other.disconnect_handler);
//    return *this;
//}
