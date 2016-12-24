#ifndef SOCKFD_H
#define SOCKFD_H
#include <iostream>
#include <assert.h>
#include <unistd.h>
#include <string.h>

class sockfd
{
    int fd;
public:
    sockfd &operator=(sockfd const&) = delete;
    sockfd(sockfd const&) = delete;
    sockfd &operator=(sockfd&&);
    sockfd(sockfd&&);

    sockfd();
    sockfd(int fd);
    ~sockfd();
    int getd();
    int dup();
};

#endif // SOCKFD_H