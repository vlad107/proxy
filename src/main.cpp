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
        port = 1555;
    } else if (argc == 2)
    {
        try
        {
            std::string port_s(argv[1]);
            size_t ptr;
            port = std::stoi(port_s, &ptr);
            if (ptr != port_s.size())
            {
                throw std::runtime_error("");
            }
        } catch (...)
        {
            std::cerr << USAGE << std::endl;
            return 1;
        }
    } else {
        std::cerr << USAGE << std::endl;
        return 1;
    }
    proxy_server server(std::make_shared<epoll_handler>());
    server.start(port);
    return 0;
}
