#ifndef SOCKFD_H
#define SOCKFD_H
#include <assert.h>
#include <unistd.h>

class sockfd
{
    int fd;
public:
    sockfd(int fd);
    ~sockfd();
};

#endif // SOCKFD_H
