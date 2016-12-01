#include "transfer_data.h"

http_buffer::http_buffer()
{
    initialize();
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
        std::string cur(std::max(0, idx - (int)SEPARATORs[i].size()), idx);
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

void http_buffer::write_all(int fd)
{
    while (true)
    {
        const size_t BUFF_SIZE = 1024;
        size_t len = std::min(BUFF_SIZE, data.size());
        std::string cur_buff(data.begin(), data.begin() + len);
        int _write = ::write(fd, cur_buff.c_str(), len);
        if (_write > 0)
        {
            data.erase(data.begin(), data.begin() + _write);
        } else break;
    }

}

host_data::host_data(std::string host)
{
    int fd = tcp_helper::open_connection(host);
    buffer_in = std::make_unique<http_buffer>();
    buffer_out = std::make_unique<http_buffer>();
}

bool host_data::empty_request()
{
    return buffer_in->empty();
}

void host_data::add_request(std::string req)
{
    buffer_in->add_chunk(req);
}

int host_data::get_server_socket()
{
    return fd;
}

void host_data::add_response(std::string resp)
{
    buffer_out->add_chunk(resp);
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

void host_data::set_response_handler(std::function<void(std::string)> f)
{
    response_handler =f;
}

transfer_data::transfer_data(int fd, std::shared_ptr<epoll_handler> efd)
{
    this->fd = fd;
    this->efd = efd;
    this->client_buffer = std::make_unique<http_buffer>();
    this->client_header = std::make_unique<http_header>();
    this->response_buffer = std::make_unique<http_buffer>();
    initialize();
}

void transfer_data::initialize()
{
    tcp_helper::make_nonblocking(fd);
    auto client_request_handler = [&](int fd)
    {   // client <fd> available for reading
        std::string chunk = tcp_helper::read_all(fd);
        client_buffer->add_chunk(chunk);
        manage_client_requests();
    };
}

void transfer_data::manage_client_requests()
{
    while (client_buffer->was_header_end())
    {
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
            if (hosts.count(host) == 0)
            {
                hosts[host] = std::make_unique<host_data>(host);
                hosts[host]->set_response_handler([&](std::string response)
                {
                    if (response_buffer->empty())
                    {
                        efd->add_event(fd, EPOLLOUT, [&](int ffd)
                        {
                            response_buffer->write_all(ffd);
                        });
                    }
                    response_buffer->add_chunk(response);
                });
            }
            auto &cur_host = hosts[host];
            if (cur_host->empty_request())
            {
                cur_host->add_writer(efd);
            }
            cur_host->add_request(req);
            efd->add_event(cur_host->get_server_socket(), EPOLLIN, [&](int fd)
            {
                cur_host->add_response(tcp_helper::read_all(fd));
            });
            client_header->clear();
        }
    }
}

int transfer_data::get_descriptor()
{
    return fd;
}
