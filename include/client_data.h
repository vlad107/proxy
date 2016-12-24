#ifndef CLIENT_DATA_H
#define CLIENT_DATA_H
#include "http_parser.h"
#include "epoll_handler.h"
#include "tcp_helper.h"
#include "sockfd.h"
#include "event_registration.h"
#include "http_buffer.h"
#include "host_data.h"

#include <unordered_map>
#include <deque>
#include <iostream>
#include <queue>
#include <functional>
#include <memory>

const int BUFF_SIZE = 1024;

const std::string BAD_REQUEST_STR = "HTTP/1.1 400 Bad Request\r\nServer: proxy\r\nContent-Length: 91\r\nContent-Type: text/html; charset=iso-8859-1\r\nConnection: Closed\r\n\r\n<html><head><title>400 Bad Request</title></head><body><h1>Bad Request</h1></body></html>\r\n";
const std::deque<char> BAD_REQUEST = tcp_helper::str_to_deque(BAD_REQUEST_STR);

class client_data
{
public:
    client_data &operator=(client_data const&) = delete;
    client_data(client_data const&) = delete;
    client_data &operator=(client_data&&) = delete;
    client_data(client_data&&) = delete;

    client_data(sockfd cfd, epoll_handler *efd);

    void data_occured(int fd);
    int get_client_infd();
    void set_disconnect(std::function<void()> disconnect_handler);
private:
    bool _was_disconnect_handler;
    epoll_handler *efd;
    sockfd client_infd;
    sockfd client_outfd;
    http_buffer request_buffer;
    http_parser request_header;
    http_buffer response_buffer;
    std::queue<std::string> result_q;
    std::unordered_map<std::string, std::unique_ptr<host_data>> hosts;
    std::function<void()> disconnect_handler;

    std::unique_ptr<event_registration> response_event;

    void return_response(std::deque<char>);
    void response_occured(const std::string &, const std::deque<char> &);
    void request_occured(const std::string &, const std::deque<char> &);
};

#endif // CLIENT_DATA_H
