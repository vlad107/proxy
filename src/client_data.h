#ifndef CLIENT_DATA_H
#define CLIENT_DATA_H
#include "buffer.h"
#include "http_header.h"
#include <fcntl.h>
#include <unistd.h>

#include <deque>
#include <iostream>
#include <memory>

const int BUFF_SIZE = 1024;

class client_data
{
    int fd;
    std::unique_ptr<buffer> buff;
    std::unique_ptr<http_header> header;
public:
    client_data(int fd);
    void read_all();
    void check_for_requests();
    int get_descriptor();
    void make_nonblocking();
};

#endif // CLIENT_DATA_H
