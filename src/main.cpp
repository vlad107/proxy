#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

#include "proxy_server.h"

const std::string USAGE = "Usage: ./server [port]";

int main(int argc, char *argv[])
{
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
