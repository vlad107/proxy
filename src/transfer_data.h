#ifndef CLIENT_DATA_H
#define CLIENT_DATA_H
#include "http_parser.h"
#include "epoll_handler.h"
#include "tcp_helper.h"
#include "sockfd.h"
#include "event_registration.h"

#include <unordered_map>
#include <deque>
#include <iostream>
#include <queue>
#include <functional>
#include <memory>

const int BUFF_SIZE = 1024;

const std::string SEPARATORs[2] = {"\r\n\r\n", "\n\n"};
const std::string BODY_END = "0\r\n\r\n";
const std::string BAD_REQUEST_STR = "HTTP/1.1 400 Bad Request\r\nServer: proxy\r\nContent-Length: 91\r\nContent-Type: text/html; charset=iso-8859-1\r\nConnection: Closed\r\n\r\n<html><head><title>400 Bad Request</title></head><body><h1>Bad Request</h1></body></html>\r\n";
const std::deque<char> BAD_REQUEST = tcp_helper::str_to_deque(BAD_REQUEST_STR);


class http_buffer
{
    std::deque<char> data;
    bool _was_header_end;
    bool _was_body_end;
    int body_end_idx;
    int header_end_idx;
    void initialize();

    void update_char(int idx);
    std::deque<char> substr(int from, int to);
public:
    http_buffer();
    void add_chunk(std::deque<char>);
    bool header_available();
    int size();
    bool empty();
    void debug_write();
    bool write_all(int fd);
    bool available_body(int body_len, bool closed);
    std::string get_header();
    std::deque<char> extract_front_http(int body_len, bool);
};

class host_data
{
    std::mutex _mutex;
    std::condition_variable cond;
    http_buffer buffer_in;
    http_buffer buffer_out;
    http_parser response_header;
    sockfd server_fdin;
    sockfd server_fdout;
    std::function<void()> disconnect_handler;
    std::function<void(int)> response_handler;
    epoll_handler *efd;
    std::unique_ptr<event_registration> response_event;
    std::shared_ptr<event_registration> request_event;
    void activate_request_handler();
    bool _started;
    bool _closed;
public:
    host_data &operator=(host_data const&) = delete;
    host_data(host_data const&) = delete;
    host_data &operator=(host_data&&) = delete;
    host_data(host_data&&) = delete;

    host_data(epoll_handler *_efd, std::function<void()>, std::function<void(int)>);
    ~host_data();
    void notify();
    void bad_request();
    void add_request(std::deque<char> req);
    void add_response(std::deque<char> resp);
    std::deque<char> extract_response();
    bool available_response();
    void start_on_socket(sockfd host_socket);
    bool closed();
    void close();
    bool empty();

    void debug_response()
    {
        buffer_out.debug_write();
    }
};

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
    http_buffer client_buffer;
    http_buffer response_buffer;
    std::unordered_map<std::string, std::unique_ptr<host_data>> hosts;

    void return_response(std::deque<char>);
    void response_occured(const std::string &, std::deque<char>);
};

#endif // CLIENT_DATA_H
