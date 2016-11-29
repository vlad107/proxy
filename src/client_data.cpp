#include "client_data.h"

client_data::client_data(int fd)
{
    this->fd = fd;
    this->buff = std::make_unique<buffer>();
    this->header = std::make_unique<http_header>();
}

void client_data::read_all()
{
    char tmp[BUFF_SIZE];
    int _read;
    while ((_read = read(fd, tmp, BUFF_SIZE)) > 0)
    {
        tmp[_read] = 0;
        std::cerr << "---\n" << tmp << "\n---" << std::endl;
        buff->add_chunk(tmp);
    }
    std::cerr << "--- current buffer---" << std::endl;
}

void client_data::check_for_requests()
{
    while (buff->was_header_end())
    {
        std::cerr << "header is available in buffer" << std::endl;
        if (header->empty()) header->parse(buff->get_header());
        int body_len = stoi(header->get_item("Content-Length"));
        std::cerr << "length of body is " << body_len << std::endl;
        if (buff->size() - buff->get_header_end() >= body_len)
        {
            std::string body = buff->get_body(body_len);
            std::cerr << "---------------------------\nReceived:\n" << body << "---------------------------\n" << std::endl;
            buff->remove_first_http_request(body_len);
        } else break;
    }
}

void client_data::make_nonblocking()
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

int client_data::get_descriptor()
{
    return fd;
}
