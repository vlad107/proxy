#include "client_data.h"

client_data::client_data(sockfd cfd, epoll_handler *efd)
    : _was_disconnect_handler(false),
      efd(efd),
      client_infd(std::move(cfd)),
      client_outfd(client_infd.dup())
{
    tcp_helper::make_nonblocking(client_infd.getd());
}

int client_data::get_client_infd()
{
    return client_infd.getd();
}

void client_data::set_disconnect(std::function<void ()> disconnect_handler)
{
    this->disconnect_handler = disconnect_handler;
    _was_disconnect_handler = true;
}

void client_data::return_response(std::deque<char> http_response, bool _closed)
{
    if (response_buffer.empty())
    {
        // TODO: is shared_ptr bad here?
        response_event = std::make_shared<event_registration>(efd,
                                                              client_outfd.getd(),
                                                              EPOLLOUT,
                                                              [&](int _fd, int _event)
        {
            if (_event & EPOLLOUT)
            {
                if (response_buffer.write_all(_fd))
                {
                    efd->add_deleter([&, this]()
                    {
                        if (response_event)
                        {
                            response_event.reset();
                        }
                        if ((result_q.empty()) || (_closed))
                        {
//                            std::cerr << "closed : " << (_closed ? 1 : 0) << std::endl;
                            assert(result_q.empty());
                            assert(_was_disconnect_handler);
                            disconnect_handler();
                        }
                    });
                }
                _event ^= EPOLLOUT;
            }
            return _event;
        });
    }
    response_buffer.add_chunk(http_response);
}

void client_data::response_occured(const std::string &host, const std::deque<char> & response)
{
    hosts[host]->add_response(response);
    while ((!result_q.empty()) && (hosts[result_q.front()]->available_response()))
    {
        std::string cur_host = result_q.front();
        result_q.pop();
        std::deque<char> http_response = hosts[cur_host]->extract_response();
        return_response(http_response, hosts[cur_host]->closed());
    }
}

void client_data::request_occured(const std::string & host, const std::deque<char> & req)
{
    if (hosts.count(host) == 0)
    {
        auto deleter_handler = [this, host]()
        {
//            std::cerr << "SERVER CLOSED" << std::endl;
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
            if ((hosts[host]->empty()) && (hosts[host]->closed()))
            {
                hosts.erase(host);
            }
        };
        hosts[host] = std::make_unique<host_data>(efd, deleter_handler, response_occured_handler);

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
        if (request_buffer.available_body(request_header, false))
        {
            std::string host(tcp_helper::normalize(request_header.get_host()));
            std::deque<char> req = request_buffer.extract_front_http(request_header);
            result_q.push(host);
            request_occured(host, req);
            request_header.clear();
        }
    }
}
