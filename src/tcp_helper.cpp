#include "tcp_helper.h"

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
    std::cerr << "start reading" << std::endl;
    while ((_read = ::read(fd, tmp, BUFF_SIZE)) > 0)
    {
        tmp[_read] = 0;
        result += tmp;
    }
    std::cerr << "finish reading" << std::endl;
    if (_read < 0)
    {
        std::cerr << "error in read():" << std::endl;
        std::cerr << std::string(strerror(errno)) << std::endl;
    }
    return result;
}

int tcp_helper::getportbyhost(std::string host)
{
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
    return port;
}

void tcp_helper::getaddrbyhost(std::string host, std::string &addr)
{
    struct hostent *tmp;
    while (1)
    {
        tmp = gethostbyname(host.c_str());
        if (tmp == nullptr)
        {
            if (errno == EINTR)
            {
                continue;
            }
            throw std::runtime_error("error in gethostbyname():\n" + std::string(strerror(errno)));
        }
        break;
    }
    addr = std::string(tmp->h_addr, tmp->h_length);
}

int tcp_helper::open_connection(std::string host_addr, int port)
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        throw std::runtime_error("error in socket():\n" + std::string(strerror(errno)));
    }
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    bcopy((char*)host_addr.c_str(), (char*)&addr.sin_addr.s_addr, host_addr.size());
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
