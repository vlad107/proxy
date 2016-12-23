#include "host_data.h"

host_data::host_data(epoll_handler *_efd,
                     std::function<void()> _disconnect_handler,
                     std::function<void(int)> _response_handler)
    : _started(false),
      disconnect_handler(_disconnect_handler),
      response_handler(_response_handler),
      efd(_efd),
      buffer_in(),
      buffer_out(),
      response_header(),
      _closed(false)
{
}

host_data::~host_data()
{
    std::unique_lock<std::mutex> lock(_mutex);
    cond.wait(lock, [this]()
    {
        return _started;
    });
}

void host_data::notify()
{
    cond.notify_all();
}

void host_data::bad_request()
{
    _started = true;
    close();
}

void host_data::close()
{
    _closed = true;
}

bool host_data::closed()
{
    return _closed;
}

void host_data::start_on_socket(sockfd host_socket)
{
    assert(!_started);
    server_fdout = std::move(host_socket);
    tcp_helper::make_nonblocking(server_fdout.getd());
    int tmpfd = dup(server_fdout.getd());
    if (tmpfd < 0)
    {
        throw std::runtime_error("error in dup()");
    }
    server_fdin = std::move(sockfd(tmpfd));
    tcp_helper::make_nonblocking(server_fdin.getd());
    response_event = std::make_unique<event_registration>(efd,
                                               server_fdin.getd(),
                                               EPOLLIN | EPOLLRDHUP,
                                               [this](int _fd, int _event)
    {
        if (_event & EPOLLRDHUP)
        {
            efd->add_deleter(disconnect_handler);
            _event ^= EPOLLRDHUP;
        } else if (_event & EPOLLIN)
        {
            response_handler(_fd);
            _event ^= EPOLLIN;
        }
        return _event;
    });
    if (!buffer_in.empty())
    {
        activate_request_handler();
    }
    _started = true;
}

bool host_data::empty()
{
    return buffer_out.empty();
}

void host_data::activate_request_handler()
{
    request_event = std::make_shared<event_registration>(efd,
                                                         server_fdout.getd(),
                                                         EPOLLOUT,
                                                         [this](int _fd, int _event)
    {
        if (_event & EPOLLOUT)
        {
            if (buffer_in.write_all(_fd))
            {
                efd->add_deleter([this]()
                {
//                    std::cerr << "deactivating request_handler" << std::endl;
                    request_event.reset();
                });
            }
            _event ^= EPOLLOUT;
        }
        return _event;
    });
}

void host_data::add_request(std::deque<char> req)
{
    if ((_started) && (buffer_in.empty()))
    {
        activate_request_handler();
    }
    buffer_in.add_chunk(req);
}

bool host_data::available_response()
{
    if (buffer_out.header_available())
    {
        if (response_header.empty())
        {
            response_header.parse_header(buffer_out.get_header(), http_parser::Direction::RESPONSE);
        }
        return buffer_out.available_body(response_header);
    }
    return false;
}

std::deque<char> host_data::extract_response()
{
    std::deque<char> result = buffer_out.extract_front_http(response_header);
    response_header.clear();
    return result;
}

void host_data::add_response(std::deque<char> resp)
{
    buffer_out.add_chunk(resp);
}

