#include "transfer_data.h"

http_buffer::http_buffer()
{
    initialize();
}

void http_buffer::debug_write()
{
    std::string s(data.begin(), data.end());
    std::cerr << "data(" << s.size() << "):\n=====" << s << "\n=====" << std::endl;
}

bool http_buffer::available_body(const http_parser &header)
{
    if (!_was_header_end) return false;
    size_t content_length = header.get_content_length();
    if (content_length == RESPONSE_CHUNKED)
    {
        return _was_body_end;
    }
    if (content_length == RESPONSE_UNTIL_END)
    {
        assert(false);
    }
    return size() - header_end_idx >= content_length;
}

void http_buffer::initialize()
{
    _was_header_end = false;
    _was_body_end = false;
    header_end_idx = -1;
    body_end_idx = -1;
    for (size_t i = 0; i < data.size(); i++)
    {
        update_char(i);
    }
}

void http_buffer::add_chunk(std::deque<char> s)
{
    data.insert(data.end(), s.begin(), s.end());
    for (size_t i = data.size() - s.size(); (i < data.size()); i++)
    {
        update_char(i);
    }
}

bool http_buffer::header_available()
{
    return _was_header_end;
}

void http_buffer::update_char(int idx)
{
    if (!_was_body_end)
    {
        int beg = std::max(0, 1 + idx - (int)BODY_END.size());
        std::string cur(data.begin() + beg, data.begin() + idx + 1);
        if (cur == BODY_END)
        {
            _was_body_end = true;
            body_end_idx = idx;
        }
    }
    if (!_was_header_end)
    {
        for (int i = 0; i < 2; i++) {
            int beg = std::max(0, 1 + idx - (int)SEPARATORs[i].size());
            std::string cur(data.begin() + beg, data.begin() + idx + 1);
            if (cur == SEPARATORs[i])
            {
                _was_header_end = true;
                header_end_idx = idx;
            }
        }
    }
}

std::deque<char> http_buffer::substr(int from, int to)
{
    if (!((0 <= from) && (from <= to) && (to <= data.size())))
    {
        throw std::out_of_range("error in buffer::substr(from, to)");
    }
    std::deque<char> result(data.begin() + from, data.begin() + to);
    return result;
}

std::deque<char> http_buffer::extract_front_http(const http_parser &header)
{
    assert(_was_header_end);
    assert(false);
}

size_t http_buffer::size()
{
    return data.size();
}

std::string http_buffer::get_header()
{
    assert(_was_header_end);
    return std::string(data.begin(), data.begin() + header_end_idx + 1);
}

bool http_buffer::empty()
{
    return (data.size() == 0);
}

bool http_buffer::write_all(int fd)
{
    while (true)
    {
        const size_t BUFF_SIZE = 1024;
        size_t len = std::min(BUFF_SIZE, data.size());
        char tmp[BUFF_SIZE];
        for (int j = 0; j < len; j++) tmp[j] = data[j];
        int _write = ::send(fd, tmp, len, MSG_NOSIGNAL);
        if (_write > 0)
        {
            data.erase(data.begin(), data.begin() + _write);
        } else if (_write < 0)
        {
            if ((errno == EINTR) || (errno == EPIPE)) continue;
            throw std::runtime_error("error in write():\n" + std::string(strerror(errno)));
        } else break;
    }
    return data.empty();
}

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
            response_header.parse_header(buffer_out.get_header(), http_parser::DIRECTION::RESPONSE);
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

transfer_data::transfer_data(sockfd cfd, epoll_handler *efd)
    : efd(efd),
      client_infd(std::move(cfd)),
      client_outfd(client_infd.dup()),
      request_buffer(),
      request_header(),
      response_buffer()
{
    tcp_helper::make_nonblocking(client_infd.getd());
}

int transfer_data::get_client_infd()
{
    return client_infd.getd();
}

void transfer_data::return_response(std::deque<char> http_response)
{
    if (response_buffer.empty())
    {
        response_event = std::make_shared<event_registration>(efd,
                                                              client_outfd.getd(),
                                                              EPOLLOUT,
                                                              [&](int _fd, int _event)
        {
            if (_event & EPOLLOUT) // todo: how to write it like a cool guy?
            {
                if (response_buffer.write_all(_fd))
                {
                    efd->add_deleter([&]()
                    {
                        response_event.reset();
                    });
                }
                _event ^= EPOLLOUT;
            }
            return _event;
        });
    }
    response_buffer.add_chunk(http_response);
}

void transfer_data::response_occured(const std::string &host, std::deque<char> response)
{
    hosts[host]->add_response(response);
    while ((!result_q.empty()) && (hosts[result_q.front()]->available_response()))
    {
        std::deque<char> http_response = hosts[result_q.front()]->extract_response();
        result_q.pop();
        return_response(http_response);
    }
}

void transfer_data::data_occured(int fd)
{
    std::cerr << "data_ocured on " << fd << std::endl;
    auto _tmp = tcp_helper::read_all(fd);
    request_buffer.add_chunk(_tmp);
    while (request_buffer.header_available())
    {
        if (request_header.empty())
        {
            request_header.parse_header(request_buffer.get_header(), http_parser::DIRECTION::REQUEST);
            assert(request_header.get_ver() != http_parser::VERSION::HTTPS);
        }
        if (request_buffer.available_body(request_header))
        {
            std::string host(tcp_helper::normalize(request_header.get_host()));
            std::deque<char> req = request_buffer.extract_front_http(request_header);
            request_header.clear();
            result_q.push(host);
            if (hosts.count(host) == 0)
            {
                hosts[host] = std::make_unique<host_data>(
                            efd,
                            [this, host]()
                {
                    hosts[host]->close();
                    response_occured(host, std::deque<char>());
                    if (hosts[host]->empty())
                    {
                        hosts.erase(host);
                    }
                },
                            [this, host](int ffd)
                {
                    std::deque<char> response = tcp_helper::read_all(ffd);
                    response_occured(host, response);
                    if ((hosts[host]->empty()) && (hosts[host]->closed()))
                    {
                        hosts.erase(host);
                    }
                });

                auto &iter = hosts[host];
                iter->add_request(req);
                efd->add_background_task([this, host, &iter]()
                {
                    int port = tcp_helper::getportbyhost(host);
                    std::string host_addr;
                    try
                    {
                        tcp_helper::getaddrbyhost(host, host_addr);
                        sockfd host_socket(tcp_helper::open_connection(host_addr, port));
                        iter->start_on_socket(std::move(host_socket));
                    } catch (...)
                    {
                        std::cerr << "BAD REQUEST" << std::endl;
                        iter->bad_request();
                        efd->add_deleter([this, host]()
                        {
                            response_occured(host, BAD_REQUEST);
                        });
                    }
                    iter->notify();
                });
            } else
            {
                hosts[host]->add_request(req);
            }
        }
    }
}
