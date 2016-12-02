#include "server_socket.h"

server_socket::server_socket(int port)
{
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0)
    {
        throw std::runtime_error("error in creating socket");
    }
    int ok = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0)
    {
        throw std::runtime_error("error in setsockopt():\n" + std::string(strerror(errno)));
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

int server_socket::accept(int fd)
{
    sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int cfd;
    while (true)
    {
        cfd = ::accept(sfd, (sockaddr*)&addr, &addr_len);
        if ((cfd < 0) && (errno == EINTR)) continue;
        break;
    }
    if (cfd < 0)
    {
        throw std::runtime_error("error in accept(server_socket)");
    }
    return cfd;
}
