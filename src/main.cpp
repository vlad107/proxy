#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <csignal>
#include <signal.h>

#include "proxy_server.h"

#define DEBUG

const std::string USAGE = "Usage: ./server [port]";

extern volatile sig_atomic_t term;

int main(int argc, char *argv[])
{
    int port;
    if (argc == 1)
    {
        port = 7777;
    } else if (argc == 2)
    {
        port = std::stoi(std::string(argv[1]));
    } else {
        std::cerr << USAGE << std::endl;
        return 1;
    }
    epoll_handler efd;
    proxy_server server(&efd, port);
    std::signal(SIGINT, [](int num)
    {
        static_cast<void>(num);
        term = 1;
    });

    efd.loop();
    return 0;
}
