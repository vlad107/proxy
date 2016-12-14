#ifndef CLIENT_DATA_H
#define CLIENT_DATA_H
#include "http_header.h"
#include "epoll_handler.h"
#include "tcp_helper.h"
#include "sockfd.h"

#include <unordered_map>
#include <deque>
#include <iostream>
#include <queue>
#include <functional>
#include <memory>

const int BUFF_SIZE = 1024;

const std::string SEPARATORs[2] = {"\r\n\r\n", "\n\n"};
const std::string HTTP_END = "0\r\n\r\n";

class http_buffer
{
    std::deque<char> data;
    bool _was_header_end;
    size_t header_end;
    void initialize();

    void update_char(int idx);
    std::string substr(int from, int to);
public:
    http_buffer();
    void add_chunk(std::string);
    bool was_header_end();
    int size();
    int get_header_end();
    bool empty();
    void debug_write();
    bool write_all(int fd);
    std::string get_header();
    std::string extract_front_http(int body_len);
};

class host_data
{
    std::unique_ptr<http_buffer> buffer_in;
    std::unique_ptr<http_buffer> buffer_out;
    std::unique_ptr<http_header> response_header;
    std::unique_ptr<sockfd> fd;
    std::unique_ptr<sockfd> fd_in;
public:
    host_data &operator=(host_data const&) = delete;
    host_data(host_data const&) = delete;
    host_data &operator=(host_data&&) = delete;
    host_data(host_data&&) = delete;

    host_data(std::string host);
    void add_request(std::string req);
    bool write_all(int fd);
    void add_response(std::string resp);
    std::string extract_response();
    bool empty_in();
    int get_out_socket();
    int get_in_socket();
    bool available_response();
    void add_writer(epoll_handler *efd);
    void set_response_handler(std::function<void(std::string)>);

    void debug_response()
    {
        buffer_out->debug_write();
    }
};

class transfer_data
{
public:
    transfer_data &operator=(transfer_data const&) = delete;
    transfer_data(transfer_data const&) = delete;
    transfer_data &operator=(transfer_data&&) = delete;
    transfer_data(transfer_data&&) = delete;

    transfer_data(int fd, epoll_handler *efd);
    void read_all();
    void check_for_requests();
    int get_descriptor();
    void make_nonblocking();
private:
    std::queue<std::string> result_q;
    void initialize();
    void manage_client_requests();
    std::unique_ptr<sockfd> fd;
    std::unique_ptr<sockfd> out_fd;
    epoll_handler *efd;
    std::unique_ptr<http_buffer> client_buffer;
    std::unique_ptr<http_header> client_header;
    std::unique_ptr<http_buffer> response_buffer;
    std::unordered_map<std::string, std::unique_ptr<host_data>> hosts;
};

#endif // CLIENT_DATA_H
