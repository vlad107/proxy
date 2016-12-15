#include "tcp_helper.h"

tcp_helper::tcp_helper()
{

}

void tcp_helper::make_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        throw std::runtime_error("error in fcntl(F_GETFL)");
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0)
    {
        throw std::runtime_error("error in fcntl(F_SETFL)");
    }
}

std::string tcp_helper::read_all(int fd)
{
    const int BUFF_SIZE = 1024;
    char tmp[BUFF_SIZE];
    int _read;
    std::string result;
    while ((_read = ::read(fd, tmp, BUFF_SIZE)) > 0)
    {
        tmp[_read] = 0;
        result += tmp;
    }
    return result;
}

int tcp_helper::open_connection(std::string host)
{
    std::cerr << "--------" << host.size() << "-------" << std::endl;
    std::cerr << host << std::endl;
    std::cerr << "----------------" << std::endl;
    int port;
    size_t i = host.find(":");
    if (i != std::string::npos)
    {
        port = std::stoi(host.substr(i + 1));
        host = host.substr(0, i);
    } else
    {
        port = 80;
    }
    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        throw std::runtime_error("error in socket():\n" + std::string(strerror(errno)));
    }
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    struct hostent *server = gethostbyname(host.c_str()); // TODO: asynchronously with epollfd
    bcopy((char*)server->h_addr, (char*)&addr.sin_addr.s_addr, server->h_length);
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        throw std::runtime_error("error in connect():\n" + std::string(strerror(errno)));
    }
    return sockfd;
}

std::string tcp_helper::normalize(std::string s)
{
    while ((!s.empty()) && ((s[s.size() - 1] == '\r') || (s[s.size() - 1] == '\n')))
    {
        s = s.substr(0, s.size() - 1);
    }
    return s;
}
