#ifndef SOCKFD_H
#define SOCKFD_H
#include <iostream>
#include <assert.h>
#include <unistd.h>

class sockfd
{
    int fd;
public:
    sockfd &operator=(sockfd const&) = delete;
    sockfd(sockfd const&) = delete;
    sockfd &operator=(sockfd&&) = delete;
    sockfd(sockfd&&) = delete;
    sockfd();
    sockfd(int fd);
    ~sockfd();
    int getd();
};

#endif // SOCKFD_H
