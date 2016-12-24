#include "proxy_server.h"

proxy_server::proxy_server(epoll_handler *efd, int port)
    : efd(efd),
      sfd(port),
      reg(efd,
          sfd.get_socket(),
          EPOLLIN,
          [this, efd](int fd, int event)
{       // TODO: dont like it
    if (event & EPOLLIN)
    {
        sockfd cfd(sfd.accept(fd));
        auto conn = std::make_unique<connection>(std::move(cfd), efd);
        auto ptr = conns.insert(std::move(conn)).first;
        (*ptr)->set_disconnect([this, ptr, efd]()
        {
            conns.erase(ptr);
        });
        (*ptr)->start();
        event ^= EPOLLIN;
    }
    return event;
})
{
    std::cerr << "Server has been started" << std::endl;
}
