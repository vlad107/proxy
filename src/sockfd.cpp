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
    if (fd > 0)
    {
        assert(close(fd) == 0);
    }
}

int sockfd::getd()
{
    return fd;
}
