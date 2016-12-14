#ifndef SOCKFD_H
#define SOCKFD_H
#include <iostream>
#include <assert.h>
#include <unistd.h>

class sockfd
{
    int fd;
public:
    sockfd();
    sockfd(int fd);
    ~sockfd();
    int getd();
};

#endif // SOCKFD_H
