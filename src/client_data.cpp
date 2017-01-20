#include "client_data.h"

client_data::client_data(sockfd cfd, epoll_handler *efd)
    : _was_disconnect_handler(false),
      efd(efd),
      client_infd(std::move(cfd)),
      client_outfd(client_infd.dup())
{
    tcp_helper::make_nonblocking(client_infd.getfd());
}

int client_data::get_client_infd()
{
    return client_infd.getfd();
}

void client_data::set_disconnect(std::function<void ()> disconnect_handler)
{
    this->disconnect_handler = disconnect_handler;
    _was_disconnect_handler = true;
}

void client_data::return_response(std::deque<char> http_response)
{
    if (response_buffer.empty())
    {
        response_event = std::make_unique<event_registration>(efd,
                                                              client_outfd.getfd(),
                                                              EPOLLOUT,
                                                              [&](int _fd, int _event)
        {
            if (_event & EPOLLOUT)
            {
                if (response_buffer.write_all(_fd))
                {
                    assert(result_q.empty());
                    assert(_was_disconnect_handler);
                    disconnect_handler();
                }
                _event ^= EPOLLOUT;
            }
            return _event;
        });
    }
    response_buffer.add_chunk(http_response);
}

void client_data::response_occured(const std::string &host, const std::deque<char> &response)
{
    std::cerr << "size = " << response.size() << std::endl;
    auto iter = hosts.find(host);
    assert(iter != hosts.end());
    iter->second->add_response(response);
    while ((!result_q.empty()) && (hosts[result_q.front()]->available_response()))
    {
        std::string cur_host = result_q.front();
        result_q.pop();
        std::deque<char> http_response = hosts[cur_host]->extract_response();
        return_response(http_response);
    }
}

void client_data::request_occured(const std::string & host, const std::deque<char> & req)
{
    if (hosts.count(host) == 0)
    {
        auto deleter_handler = [this, host]()
        {
            hosts[host]->close();
            response_occured(host, std::deque<char>());
            if (hosts[host]->empty())
            {
                hosts.erase(host);
            }
        };
        auto response_occured_handler = [this, host](int ffd)
        {
            std::deque<char> response = tcp_helper::read_all(ffd);
            response_occured(host, response);
            if (hosts[host]->empty())
            {
                hosts.erase(host);
            }
        };
        int _ev = eventfd(0, O_CLOEXEC);
        auto shared_host = std::make_shared<sockfd>();
        auto handler = [_ev, this, host, shared_host](int _fd, int _event)
        {
            std::cerr << "CONNECTION OPENED ON " << shared_host->getfd() << std::endl;
            eventfd_t flag;
            int err = eventfd_read(_fd, &flag);
            assert(err == 0);
            if (_event & EPOLLIN)
            {
                if (flag == 107)
                {
                    assert(shared_host->getfd() != -1);
                    hosts[host]->start_on_socket(std::move(*shared_host));
                } else if (flag == 108)
                {
                    hosts[host]->bad_request();
                    response_occured(host, BAD_REQUEST);
                }
                hosts[host]->notify();
                wait_regs.erase(wait_regs.find(host));
                _event ^= EPOLLIN;
            }
            std::cerr << "_event = " << _event << std::endl;
            return _event;
        };
        wait_regs[host] = std::make_unique<event_registration>(efd, _ev, EPOLLIN, handler);
        hosts[host] = std::make_unique<host_data>(efd, deleter_handler, response_occured_handler);
        hosts[host]->add_request(req);
        efd->add_background_task([shared_host, this, host, _ev]()
        {
            int port = tcp_helper::getportbyhost(host);
            std::string host_addr;
            try
            {
                tcp_helper::getaddrbyhost(host, host_addr);
                shared_host->setfd(tcp_helper::open_connection(host_addr, port));
                int err = eventfd_write(_ev, 107);
                std::cerr << "writed 0 to " << _ev << std::endl << "  err = " << err << std::endl;
                assert(err == 0);
            } catch (...)
            {
                std::cerr << "BAD REQUEST" << std::endl;
                int err = eventfd_write(_ev, 108);
                assert(err == 0);
            }
        });
    } else
    {
        hosts[host]->add_request(req);
    }
}

void client_data::data_occured(int fd)
{
    std::cerr << "data_ocured on " << fd << std::endl;
    request_buffer.add_chunk(tcp_helper::read_all(fd));
    while (request_buffer.header_available())
    {
        std::cerr << "NEW REQUEST HEADER AVAILABLE" << std::endl;
        if (request_header.empty())
        {
            request_header.parse_header(request_buffer.get_header(), http_parser::Direction::REQUEST);
            assert(request_header.get_ver() != http_parser::Version::HTTPS);
        }
        if (request_buffer.body_available(request_header, false))
        {
            std::string host(tcp_helper::normalize(request_header.get_host()));
            std::deque<char> req = request_buffer.extract_front_http(request_header);
            result_q.push(host);
            request_occured(host, req);
            request_header.clear();
        }
    }
}
