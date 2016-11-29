#ifndef BUFFER_H
#define BUFFER_H
#include <deque>
#include <string>
#include <stdexcept>

const std::string SEPARATORs[2] = {"\r\n\r\n", "\n\n"};

class buffer
{

    std::deque<char> data;
    size_t sep_idx[2];
    bool _was_header_end;
    int header_end;
    void initialize();

    void update_char(char c, int idx);
    std::string substr(int from, int to);
public:
    buffer();
    void add_chunk(std::string);
    bool was_header_end();
    int size();
    int get_header_end();

    void remove_first_http_request(int body_len);
    std::string get_header();
    std::string get_body(int body_len);
};

#endif // BUFFER_H
