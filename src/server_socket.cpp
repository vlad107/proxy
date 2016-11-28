#include "server_socket.h"

server_socket::server_socket(int port)
{
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0)
    {
        throw std::runtime_error("error in creating socket");
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sfd, (struct sockaddr*) &addr, sizeof(struct sockaddr)) < 0)
    {
        throw std::runtime_error("error in bind()");
    }
    if (listen(sfd, BACKLOG) < 0)
    {
        throw std::runtime_error("error in listen()");
    }
}

server_socket::~server_socket()
{
    close(sfd);
}

int server_socket::get_socket()
{
    return sfd;
}
