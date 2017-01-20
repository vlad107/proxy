#include "sockfd.h"

sockfd::sockfd() : fd(-1)
{
}

sockfd::sockfd(int fd) : fd(fd)
{
}

sockfd::~sockfd()
{    if (fd != -1)
    {
        int err = close(fd);
        assert(err == 0);
    }
}

int sockfd::getfd()
{
    return fd;
}

int sockfd::setfd(int fd)
{
    if (this->fd != -1)
    {
        return -1;
    }
    this->fd = fd;
    return 0;
}

sockfd::sockfd(sockfd &&other)
{
    fd = other.fd;
    other.fd = -1;
}

sockfd &sockfd::operator=(sockfd &&other)
{
    std::swap(fd, other.fd);
    return *this;
}

int sockfd::dup()
{
    int tmp = ::dup(fd);
    if (tmp < 0)
    {
        throw std::runtime_error("error in dup(" + std::to_string(fd) + "):\n" + std::string(strerror(errno)));
    }
    return tmp;
}
