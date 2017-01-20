#ifndef HOST_DATA_H
#define HOST_DATA_H
#include <stdio.h>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <functional>

#include "http_buffer.h"
#include "http_parser.h"
#include "sockfd.h"
#include "epoll_handler.h"
#include "event_registration.h"
#include "tcp_helper.h"

class host_data
{
    epoll_handler *efd;
    std::function<void()> disconnect_handler;
    std::function<void(int)> response_handler;
    bool _started;
    bool _closed;

    std::mutex _mutex;
    std::condition_variable cond;
    http_buffer buffer_in;
    http_buffer buffer_out;
    http_parser response_header;
    sockfd server_fdin;
    sockfd server_fdout;
    event_registration response_event;

    std::unique_ptr<event_registration> request_event;
    void activate_request_handler();
public:
    host_data &operator=(host_data const&) = delete;
    host_data(host_data const&) = delete;
    host_data &operator=(host_data&&) = delete;
    host_data(host_data&&) = delete;

    host_data(epoll_handler *_efd, std::function<void()>, std::function<void(int)>);
    ~host_data();

    bool response_available();
    std::deque<char> extract_response();


    void notify();
    void bad_request();
    void add_request(std::deque<char> req);
    void add_response(std::deque<char> resp);
    void start_on_socket(sockfd host_socket);
    bool closed();
    void close();
    bool empty();

};

#endif // HOST_DATA_H
