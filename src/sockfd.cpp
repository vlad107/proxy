#include "sockfd.h"

sockfd::sockfd() : fd(-1)
{
}

sockfd::sockfd(int fd) : fd(fd)
{
}

sockfd::~sockfd()
{
    std::cerr << "closing socket " << fd << std::endl;
    if (fd != -1)
    {
        int err = close(fd);
        assert(err == 0);
    }
}

int sockfd::getd()
{
    return fd;
}

sockfd::sockfd(sockfd &&other)
{
    fd = other.fd;
    other.fd = -1;
}

sockfd &sockfd::operator=(sockfd &&other) // TODO: swap-trick should be here
{
    fd = other.fd;
    other.fd = -1;
//    std::swap(fd, other.fd);
    return *this;
}
