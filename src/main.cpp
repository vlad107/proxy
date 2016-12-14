#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <signal.h>

#include "proxy_server.h"

const std::string USAGE = "Usage: ./server [port]";

void handler(int num)
{
    std::cerr << "Termination..." << std::endl;
    exit(0);
}

void set_sigactions()
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    sigaction(SIGINT, &act, nullptr);
    sigaction(SIGTERM, &act, nullptr);
}

int main(int argc, char *argv[])
{
    set_sigactions();
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
    efd.loop();
    return 0;
}
