#include "host_data.h"

host_data::host_data(epoll_handler *efd,
                     std::function<void()> disconnect_handler,
                     std::function<void(int)> response_handler)
    : efd(efd),
      disconnect_handler(disconnect_handler),
      response_handler(response_handler),
      cur_state(State::BEFORE)
{
}

host_data::~host_data()
{
//    std::cerr << "waiting..." << std::endl;
//    std::unique_lock<std::mutex> lock(_mutex);
//    cond.wait(lock, [this]()
//    {
//        return _started;
//    });
//    std::cerr << "descructed" << std::endl;
}

void host_data::notify()
{
    cond.notify_all();
}

void host_data::bad_request()
{
    cur_state = State::CLOSED;
}

void host_data::start_on_socket(sockfd host_socket)
{
    assert(cur_state == State::BEFORE);
    server_fdout = std::move(host_socket);
    tcp_helper::make_nonblocking(server_fdout.getfd());
    int tmpfd = dup(server_fdout.getfd());
    if (tmpfd < 0)
    {
        throw std::runtime_error("error in dup()");
    }
    server_fdin = std::move(sockfd(tmpfd));
    tcp_helper::make_nonblocking(server_fdin.getfd());
    std::cerr << "NEW EVENT_REGISTRATION ON " << server_fdin.getfd() << std::endl;
    response_event = std::move(
                event_registration(efd,
                                   server_fdin.getfd(),
                                   EPOLLIN | EPOLLRDHUP,
                                   [this](int _fd, int _event)
    {
        if (_event & EPOLLIN)
        {
            std::cerr << "EPOLLIN ON " << _fd << std::endl;
            response_handler(_fd);
            _event ^= EPOLLIN;
        }
        if (_event & EPOLLRDHUP)
        {
            std::cerr << "EPOLLRDHUP ON " << _fd << std::endl;
            disconnect_handler();
            _event ^= EPOLLRDHUP;
        }
        return _event;
    }));
    if (!buffer_in.empty())
    {
        activate_request_handler();
    }
    cur_state = State::STARTED;
}

bool host_data::empty()
{
    return buffer_out.empty();
}

void host_data::close()
{
    cur_state = State::CLOSED;
}

void host_data::activate_request_handler()
{
    request_event = std::make_unique<event_registration>(efd,
                                                         server_fdout.getfd(),
                                                         EPOLLOUT,
                                                         [this](int _fd, int _event)
    {
        if (_event & EPOLLOUT)
        {
            if (buffer_in.write_all(_fd))
            {
                if (request_event)
                {
                    request_event.reset();
                }
            }
            _event ^= EPOLLOUT;
        }
        return _event;
    });
}

void host_data::add_request(std::deque<char> req)
{
    if ((cur_state == State::STARTED) && (buffer_in.empty()))
    {
        activate_request_handler();
    }
    buffer_in.add_chunk(req);
}

bool host_data::response_available()
{
    if (buffer_out.header_available())
    {
        if (response_header.empty())
        {
            response_header.parse_header(buffer_out.get_header(), http_parser::Direction::RESPONSE);
        }
        return buffer_out.body_available(response_header, cur_state == State::CLOSED);
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

