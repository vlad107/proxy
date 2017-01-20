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
    size_t body_end_idx;
    size_t header_end_idx;
    void initialize();

    void update_char(size_t idx);
    std::deque<char> substr(size_t from, size_t to);
    bool equals(size_t idx, const std::string s);
public:
    http_buffer();
    void add_chunk(std::deque<char>);
    bool header_available();
    size_t size();
    bool empty();
    bool write_all(int fd);
    bool body_available(const http_parser & header, bool started);
    std::string get_header();
    std::deque<char> extract_front_http(const http_parser &header);
};

#endif // HTTP_BUFFER_H
