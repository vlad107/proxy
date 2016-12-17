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
#include <deque>
#include <memory>

namespace tcp_helper
{
    int getportbyhost(std::string);
    void getaddrbyhost(std::string, std::string &server);
    void make_nonblocking(int fd);
    std::deque<char> read_all(int fd);
    int open_connection(std::string, int port);
    std::string normalize(std::string host);
}

#endif // TCP_HELPER_H
