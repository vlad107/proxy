#ifndef HTTP_BUFFER_H
#define HTTP_BUFFER_H
#include <deque>
#include <string>
#include <iostream>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "http_parser.h"

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
    size_t size();
    bool empty();
    bool write_all(int fd);
    bool available_body(const http_parser & header, bool started);
    std::string get_header();
    std::deque<char> extract_front_http(const http_parser &header);
};

#endif // HTTP_BUFFER_H
