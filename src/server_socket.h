#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <memory.h>

#include "sockfd.h"

class server_socket
{
    std::unique_ptr<sockfd> sfd;
    static const int BACKLOG = 32;
public:
    server_socket &operator=(server_socket const&) = delete;
    server_socket(server_socket const&) = delete;
    server_socket &operator=(server_socket&&) = delete;
    server_socket(server_socket&&) = delete;

    server_socket(int port);
    int get_socket();
    int accept(int fd);
};

#endif // SERVER_SOCKET_H
