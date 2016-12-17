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

void http_buffer::initialize()
{
    _was_header_end = false;
    header_end = -1;
    for (size_t i = 0; i < data.size(); i++)
    {
        if (_was_header_end) break;
        update_char(i);
    }
}

void http_buffer::add_chunk(std::string s)
{
//    std::cerr << "adding:\n=====\n" << s << "\n====\nto buffer" << std::endl;
    data.insert(data.end(), s.begin(), s.end());
    for (size_t i = data.size() - s.size(); i < data.size(); i++)
    {
        if (_was_header_end) break;
        update_char(i);
    }
}

bool http_buffer::header_available()
{
    return _was_header_end;
}

void http_buffer::update_char(int idx)
{
    for (int i = 0; i < 2; i++) {
        int beg = std::max(0, 1 + idx - (int)SEPARATORs[i].size());
        std::string cur(data.begin() + beg, data.begin() + idx + 1);
        if (cur == SEPARATORs[i])
        {
            _was_header_end = true;
            header_end = idx;
        }
    }
}

int http_buffer::get_header_end()
{
    return header_end;
}

std::string http_buffer::substr(int from, int to)
{
    if (!((0 <= from) && (from <= to) && (to <= data.size())))
    {
        throw std::out_of_range("error in buffer::substr(from, to)");
    }
    std::string result(data.begin() + from, data.begin() + to);
    return result;
}

std::string http_buffer::extract_front_http(int body_len)
{
    std::string result = substr(0, header_end + 1 + body_len);
    data.erase(data.begin(), data.begin() + header_end + 1 + body_len);
    initialize();
    return result;
}

int http_buffer::size()
{
    return data.size();
}

std::string http_buffer::get_header()
{
    return substr(0, header_end + 1);
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
        std::string cur_buff(data.begin(), data.begin() + len);
        int _write = ::write(fd, cur_buff.c_str(), len);
//        std::cerr << len << " of " << data.size() << " was written to descriptor " << fd << std::endl;
        if (_write > 0)
        {
//            std::cerr << "======" << std::endl;
//            std::cerr << cur_buff.substr(0, _write);
//            std::cerr << "======" << std::endl;
            data.erase(data.begin(), data.begin() + _write);
        } else if (_write < 0)
        {
            std::cerr << "error in write():\n";
            std::cerr << strerror(errno) << std::endl;
        } else break;
    }
    return data.empty();
}

host_data::host_data(epoll_handler *_efd,
                     std::function<void()> _disconnect_handler,
                     std::function<void(int)> _response_handler)
{
    _started = false;
    disconnect_handler = _disconnect_handler;
    response_handler = _response_handler;
    efd = _efd;
    std::cerr << "opening connection" << std::endl;
    buffer_in = std::make_unique<http_buffer>();
    buffer_out = std::make_unique<http_buffer>();
    response_header = std::make_unique<http_parser>();
}

void host_data::start_on_socket(sockfd host_socket)
{
    assert(!_started);
    _started = true;
    server_fdout = std::move(host_socket);
    std::cerr << "host opened on " << server_fdout.getd() << " descriptor" << std::endl;
    tcp_helper::make_nonblocking(server_fdout.getd());
    int tmpfd = dup(server_fdout.getd());
    if (tmpfd < 0)
    {
        throw std::runtime_error("error in dup()");
    }
    server_fdin = std::move(sockfd(tmpfd));
    std::cerr << "second descriptor for host is " << server_fdin.getd() << std::endl;
    tcp_helper::make_nonblocking(server_fdin.getd());
    response_event = std::make_unique<event_registration>(efd,
                                               server_fdin.getd(),
                                               EPOLLIN | EPOLLRDHUP,
                                               [this](int _fd, int _event)
    {
        if (_event & EPOLLIN)
        {
            response_handler(_fd);
            _event ^= EPOLLIN;
        }
        if (_event & EPOLLRDHUP)
        {
            efd->add_deleter(disconnect_handler);
            _event ^= EPOLLRDHUP;
        }
        assert(_event == 0);
    });
    if (!buffer_in->empty())
    {
        activate_request_handler();
    }
}

void host_data::activate_request_handler()
{
    std::cerr << "activating request_handler" << std::endl;
    request_event = std::make_shared<event_registration>(efd,
                                                         server_fdout.getd(),
                                                         EPOLLOUT,
                                                         [this](int _fd, int _event)
    {
        if (_event & EPOLLOUT)
        {
            if (buffer_in->write_all(_fd))
            {
                efd->add_deleter([this]()
                {
                    std::cerr << "deactivating request_handler" << std::endl;
                    request_event.reset();
                });
            }
            _event ^= EPOLLOUT;
        }
        assert(_event == 0);
    });
}

void host_data::add_request(std::string req)
{
    if ((_started) && (buffer_in->empty()))
    {
        activate_request_handler();
    }
    buffer_in->add_chunk(req);
}

bool host_data::available_response()
{
    if (buffer_out->header_available())
    {
        if (response_header->empty())
        {
            response_header->parse_header(buffer_out->get_header());
        }
        int available_len = buffer_out->size() - buffer_out->get_header_end();
        int body_len = response_header->get_content_len();
        if (available_len >= body_len)
        {
            return true;
        }
    }
    return false;
}

std::string host_data::extract_response()
{
    int body_len = response_header->get_content_len();
    std::string result = buffer_out->extract_front_http(body_len);
    response_header->clear();
    return result;
}

void host_data::add_response(std::string resp)
{
    std::cerr << "adding response" << std::endl;
    buffer_out->add_chunk(resp);
    std::cerr << "response was added" << std::endl;
}

transfer_data::transfer_data(int _fd, epoll_handler *_efd)
{
    client_infd = std::make_unique<sockfd>(_fd);
    int tmpfd = dup(_fd);
    if (tmpfd < 0)
    {
        throw std::runtime_error("error in dup()");
    }
    client_outfd = std::make_unique<sockfd>(tmpfd);
    efd = _efd;
    client_buffer = std::make_unique<http_buffer>();
    request_header = std::make_unique<http_parser>();
    response_buffer = std::make_unique<http_buffer>();
    tcp_helper::make_nonblocking(client_infd->getd());
}

void transfer_data::data_occured(int fd)
{
    client_buffer->add_chunk(tcp_helper::read_all(fd));
//    client_buffer->debug_write();
    while (client_buffer->header_available())
    {
        std::cerr << "request_header available" << std::endl;
        if (request_header->empty())
        {
            request_header->parse_header(client_buffer->get_header());
        }
        int body_len = request_header->get_content_len();
        std::string host = request_header->get_host();
        int available_len = client_buffer->size() - client_buffer->get_header_end();
        if (available_len >= body_len)
        {
            request_header->clear();

            std::cerr << "full http-request available" << std::endl;
            std::string req = client_buffer->extract_front_http(body_len);
            host = tcp_helper::normalize(host);
            result_q.push(host);
            if (hosts.count(host) == 0)
            {
                hosts[host] = std::make_unique<host_data>(
                            efd,
                            [this, host]()
                {
                    hosts.erase(host);
                },
                            [this, host](int ffd)
                {
                    std::string response = tcp_helper::read_all(ffd);
                    hosts[host]->add_response(response);
                    while ((!result_q.empty()) && (hosts[result_q.front()]->available_response()))
                    {
                        std::string http_response = hosts[result_q.front()]->extract_response();
                        result_q.pop();
                        if (response_buffer->empty())
                        {
                            response_event = std::make_shared<event_registration>(efd,
                                                                                  client_outfd->getd(),
                                                                                  EPOLLOUT,
                                                                                  [&](int _fd, int _event)
                            {
                                if (_event & EPOLLOUT)
                                {
                                    if (response_buffer->write_all(_fd))
                                    {
                                        efd->add_deleter([&]()
                                        {
                                            response_event.reset();
                                        });
                                    }
                                    _event ^= EPOLLOUT;
                                }
                                assert(_event == 0);
                            });
                        }
                        response_buffer->add_chunk(http_response);
                    }
                });

                hosts[host]->add_request(req);
                auto &iter = hosts[host];
                efd->add_background_task([this, host, &iter]()
                {
                    std::cerr << "host: " << host << std::endl;
                    int port = tcp_helper::getportbyhost(host);
                    std::cerr << "   port: " << port << std::endl;
                    std::string host_addr;
                    tcp_helper::getaddrbyhost(host, host_addr);
                    std::cerr << "   addr: " << host_addr << std::endl;
                    sockfd host_socket(tcp_helper::open_connection(host_addr, port));
                    std::cerr << "   connection opened" << std::endl;
                    // start_on_socket should block iter
                    iter->start_on_socket(std::move(host_socket));
                });
            } else
            {
                hosts[host]->add_request(req);
            }
        }
    }
}
