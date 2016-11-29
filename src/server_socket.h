#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <memory.h>

class server_socket
{
    int sfd;
    static const int BACKLOG = 32;
public:
    server_socket(int port);
    ~server_socket();
    int get_socket();
    int accept(int fd);
};

#endif // SERVER_SOCKET_H
