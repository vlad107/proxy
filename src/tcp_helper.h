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

class tcp_helper
{
public:
    static void make_nonblocking(int fd);
    static std::string read_all(int fd);
    static int open_connection(std::string host);
    static std::string normalize(std::string host);
    tcp_helper();
};

#endif // TCP_HELPER_H
