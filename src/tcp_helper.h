#ifndef TCP_HELPER_H
#define TCP_HELPER_H
#include <fcntl.h>
#include <unistd.h>

#include <string>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <iostream>

namespace tcp_helper
{
    void make_nonblocking(int fd);
    std::string read_all(int fd);
    int open_connection(std::string host);
    std::string normalize(std::string host);
}

#endif // TCP_HELPER_H
