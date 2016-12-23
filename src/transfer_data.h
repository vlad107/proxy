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

class transfer_data
{
public:
    transfer_data &operator=(transfer_data const&) = delete;
    transfer_data(transfer_data const&) = delete;
    transfer_data &operator=(transfer_data&&) = delete;
    transfer_data(transfer_data&&) = delete;

    transfer_data(sockfd cfd, epoll_handler *efd);
    void data_occured(int fd);
    int get_client_infd();
private:
    std::shared_ptr<event_registration> response_event;
    std::queue<std::string> result_q;
    epoll_handler *efd;
    sockfd client_infd;
    sockfd client_outfd;

    http_parser request_header;
    http_buffer request_buffer;
    http_buffer response_buffer;
    std::unordered_map<std::string, std::unique_ptr<host_data>> hosts;

    void return_response(std::deque<char>);
    void response_occured(const std::string &, std::deque<char>);
};

#endif // CLIENT_DATA_H
