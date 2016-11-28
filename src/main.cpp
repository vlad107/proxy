#include <iostream>
#include <string>
#include <sstream>
#include "hints.h"
#include "proxy_server.h"

const std::string USAGE = "Usage: ./server [port]";

int main(int argc, char *argv[])
{
    int port;
    if (argc == 1)
    {
        port = 8080;
    } else if (argc == 2)
    {
        try
        {
            port = string_to_int(argv[1]);
        } catch (const std::bad_cast& e)
        {
            std::cerr << USAGE << std::endl;
            return 1;
        }
    } else {
        std::cerr << USAGE << std::endl;
        return 1;
    }
    proxy_server server(std::make_unique<epoll_handler>());
    server.start(port);
    return 0;
}
