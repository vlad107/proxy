#include "client_data.h"

client_data::client_data(sockfd cfd, epoll_handler *efd)
    : efd(efd),
      client_infd(std::move(cfd)),
      client_outfd(client_infd.dup())
{
    tcp_helper::make_nonblocking(client_infd.getd());
}

int client_data::get_client_infd()
{
    return client_infd.getd();
}

void client_data::return_response(std::deque<char> http_response)
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

void client_data::response_occured(const std::string &host, std::deque<char> response)
{
    hosts[host]->add_response(response);
    while ((!result_q.empty()) && (hosts[result_q.front()]->available_response()))
    {
        std::deque<char> http_response = hosts[result_q.front()]->extract_response();
        result_q.pop();
        return_response(http_response);
    }
}

void client_data::data_occured(int fd)
{
    std::cerr << "data_ocured on " << fd << std::endl;
    request_buffer.add_chunk(tcp_helper::read_all(fd));
    while (request_buffer.header_available())
    {
        if (request_header.empty())
        {
            request_header.parse_header(request_buffer.get_header(), http_parser::Direction::REQUEST);
            assert(request_header.get_ver() != http_parser::Version::HTTPS);
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
