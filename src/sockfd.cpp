#include "sockfd.h"

sockfd::sockfd(int fd) : fd(fd)
{
}

sockfd::~sockfd()
{
    assert(close(fd) == 0);
}
