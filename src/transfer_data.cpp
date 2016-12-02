#include "transfer_data.h"

http_buffer::http_buffer()
{
    initialize();
}

void http_buffer::debug_write()
{
    std::string s(data.begin(), data.end());
    std::cerr << "data(" << s.size() << "):\n=====" << s << "\n=====" << std::endl;
}

void http_buffer::initialize()
{
    _was_header_end = false;
    header_end = -1;
    for (size_t i = 0; i < data.size(); i++)
    {
        if (_was_header_end) break;
        update_char(i);
    }
}

void http_buffer::add_chunk(std::string s)
{
    std::cerr << "adding:\n=====\n" << s << "\n====\nto buffer" << std::endl;
    data.insert(data.end(), s.begin(), s.end());
    for (size_t i = data.size() - s.size(); i < data.size(); i++)
    {
        if (_was_header_end) break;
        update_char(i);
    }
}

bool http_buffer::was_header_end()
{
    return _was_header_end;
}

void http_buffer::update_char(int idx)
{
    for (int i = 0; i < 2; i++) {
        int beg = std::max(0, 1 + idx - (int)SEPARATORs[i].size());
        std::string cur(data.begin() + beg, data.begin() + idx + 1);
        if (cur == SEPARATORs[i])
        {
            _was_header_end = true;
            header_end = idx;
        }
    }
}

int http_buffer::get_header_end()
{
    return header_end;
}

std::string http_buffer::substr(int from, int to)
{
    if (!((0 <= from) && (from <= to) && (to <= data.size())))
    {
        throw std::out_of_range("error in buffer::substr(from, to)");
    }
    std::string result(data.begin() + from, data.begin() + to);
    return result;
}

std::string http_buffer::extract_front_http(int body_len)
{
    std::string result = substr(0, header_end + 1 + body_len);
    data.erase(data.begin(), data.begin() + header_end + 1 + body_len);
    initialize();
    return result;
}

int http_buffer::size()
{
    return data.size();
}

std::string http_buffer::get_header()
{
    return substr(0, header_end + 1);
}

bool http_buffer::empty()
{
    return (data.size() == 0);
}

bool http_buffer::write_all(int fd)
{
    while (true)
    {
        const size_t BUFF_SIZE = 1024;
        size_t len = std::min(BUFF_SIZE, data.size());
        std::string cur_buff(data.begin(), data.begin() + len);
        int _write = ::write(fd, cur_buff.c_str(), len);
        std::cerr << len << " of " << data.size() << " was written to descriptor " << fd << std::endl;
        if (_write > 0)
        {
            std::cerr << "======" << std::endl;
            std::cerr << cur_buff.substr(0, _write);
            std::cerr << "======" << std::endl;
            data.erase(data.begin(), data.begin() + _write);
        } else if (_write < 0)
        {
            std::cerr << "error in write():\n";
            std::cerr << strerror(errno) << std::endl;
        } else break;
    }
    return data.empty();
}

host_data::host_data(std::string host)
{
    std::cerr << "opening connection" << std::endl;
    fd = tcp_helper::open_connection(host);
    tcp_helper::make_nonblocking(fd);
    std::cout << "connection opened to descriptor " << fd << std::endl;
    fd_in = dup(fd);
    tcp_helper::make_nonblocking(fd_in);
    std::cerr << "descriptor for writing to host " << host << " is " << fd << std::endl;
    std::cerr << "descriptor for reading from host " << host << " is " << fd_in << std::endl;
    buffer_in = std::make_unique<http_buffer>();
    buffer_out = std::make_unique<http_buffer>();
    response_header = std::make_unique<http_header>();
}

bool host_data::empty_in()
{
    return buffer_in->empty();
}

void host_data::add_request(std::string req)
{
    buffer_in->add_chunk(req);
}

bool host_data::write_all(int fd)
{
    return buffer_in->write_all(fd);
}

int host_data::get_out_socket()
{
    return fd;
}

int host_data::get_in_socket()
{
    return fd_in;
}

bool host_data::available_response()
{
    if (buffer_out->was_header_end())
    {
        if (response_header->empty())
        {
            response_header->parse(buffer_out->get_header());
        }
        int available_len = buffer_out->size() - buffer_out->get_header_end();
        int body_len = response_header->get_content_len();
        if (available_len >= body_len)
        {
            return true;
        }
    }
    return false;
}

std::string host_data::extract_response()
{
    int body_len = response_header->get_content_len();
    std::string result = buffer_out->extract_front_http(body_len);
    response_header->clear();
    return result;
}

void host_data::add_response(std::string resp)
{
    std::cerr << "adding response" << std::endl;
    buffer_out->add_chunk(resp);
    std::cerr << "response was added" << std::endl;
}

void host_data::add_writer(std::shared_ptr<epoll_handler> efd)
{
    efd->add_event(fd, EPOLLOUT, [&](int fd)
    {
        buffer_in->write_all(fd);
        if (buffer_in->empty())
        {
            efd->rem_event(fd, EPOLLOUT);
        }
    });
}

transfer_data::transfer_data(int fd, std::shared_ptr<epoll_handler> efd)
{
    this->fd = fd;
    this->out_fd = dup(fd);
    this->efd = efd;
    this->client_buffer = std::make_unique<http_buffer>();
    this->client_header = std::make_unique<http_header>();
    this->response_buffer = std::make_unique<http_buffer>();
    initialize();
}

void transfer_data::initialize()
{
    tcp_helper::make_nonblocking(fd);
    efd->add_event(fd, EPOLLIN, [&](int fd)
    {
        std::cerr << "new data on " << fd << " descriptor" << std::endl;
        client_buffer->add_chunk(tcp_helper::read_all(fd));
        client_buffer->debug_write();
        manage_client_requests();
    });
}

void transfer_data::manage_client_requests()
{
    while (client_buffer->was_header_end())
    {
        std::cerr << "header detected!" << std::endl;
        if (client_header->empty())
        {
            client_header->parse(client_buffer->get_header());
        }
        int body_len = client_header->get_content_len();
        int available_len = client_buffer->size() - client_buffer->get_header_end();
        if (available_len >= body_len)
        {
            std::string req = client_buffer->extract_front_http(body_len);
            std::string host = client_header->get_host(); // TODO: or IP?
            host = tcp_helper::normalize(host);
            std::cerr << "==host===" << std::endl;
            std::cerr << host << std::endl;
            std::cerr << "==\\host===" << std::endl;
            result_q.push(host);
            if (hosts.count(host) == 0)
            {
                hosts[host] = std::make_unique<host_data>(host);
                std::cerr << "--it is host!!!--" << std::endl;
                std::cerr << host << std::endl;
                std::cerr << "--the end of the host--" << std::endl;
                std::cerr << "add event onRead for host" << std::endl;
                efd->add_event(hosts[host]->get_in_socket(), EPOLLIN, [&, host, this](int ffd)
                {
                    std::cerr << "response from descriptor " << ffd << " is available" << std::endl;
                    std::string response = tcp_helper::read_all(ffd);
                    std::cerr << "====" << std::endl;
                    std::cerr << response << std::endl;
                    std::cerr << "====" << std::endl;
                    std::cerr << "--it is host!!!--" << std::endl;
                    std::cerr << host << std::endl;
                    std::cerr << "--the end of the host--" << std::endl;
                    hosts[host]->add_response(response);
                    std::cerr << result_q.front() << std::endl;
                    while ((!result_q.empty()) && (hosts[result_q.front()]->available_response()))
                    {
                        std::cerr << "some response is available" << std::endl;
                        std::string http_response = hosts[result_q.front()]->extract_response();
                        result_q.pop();
                        if (response_buffer->empty())
                        {
                            std::cerr << "add event onRead for client" << std::endl;
                            efd->add_event(out_fd, EPOLLOUT, [&](int out_fd)
                            {
                                std::cerr << "client can read" << std::endl;
                                if (response_buffer->write_all(out_fd))
                                {
                                    efd->rem_event(out_fd, EPOLLOUT);
                                }
                            });
                        }
                        std::cerr << "adding chunk..." << std::endl;
                        response_buffer->add_chunk(response);
                        std::cerr << "added" << std::endl;
                    }
                });
            }
            auto &cur_host = hosts[host];
            if (cur_host->empty_in())
            {
                std::cerr << "add onWrite for host" << std::endl;
                efd->add_event(cur_host->get_out_socket(), EPOLLOUT, [&](int out_fd)
                {
                    std::cerr << "original server can read" << std::endl;
                    if (cur_host->write_all(out_fd))
                    {
                        std::cerr << "removing..." << std::endl;
                        efd->rem_event(cur_host->get_out_socket(), EPOLLOUT);
                    }

                });
            }
            cur_host->add_request(req);
            client_header->clear();
        }
    }
}

int transfer_data::get_descriptor()
{
    return fd;
}
