#include "proxy_server.h"

proxy_server::proxy_server(epoll_handler *efd, int port)
    : efd(efd),
      sfd(port),
      reg(efd,
          sfd.get_socket(),
          EPOLLIN,
          [this, efd](int fd, int event)
{
    if (event & EPOLLIN)
    {
        std::cerr << "NEW CLIENT DETECTED" << std::endl;
        sockfd cfd(sfd.accept(fd));
        int index = cfd.getd();
        auto ptr = conns.emplace(index, std::make_unique<connection>(std::move(cfd), efd)).first;
        ptr->second->set_disconnect([this, ptr]()
        {
            conns.erase(ptr);
        });
        ptr->second->start();
        event ^= EPOLLIN;
    }
    return event;
})
{
    std::cerr << "Server has been started" << std::endl;
}
