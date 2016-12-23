#include "http_buffer.h"

http_buffer::http_buffer()
{
    initialize();
}

bool http_buffer::available_body(const http_parser &header, bool started)
{
    assert(_was_header_end);
    size_t content_length = header.get_content_length();
    if (content_length == RESPONSE_CHUNKED)
    {
        return _was_body_end;
    }
    if (content_length == RESPONSE_UNTIL_END)
    {
        assert(header.get_dir() == http_parser::Direction::RESPONSE); // TODO: if client is HTTP/1.0 then it's possible for request
        return !started;
    }
    return size() - header_end_idx >= content_length;
}

void http_buffer::initialize()
{
    _was_header_end = false;
    _was_body_end = false;
    header_end_idx = -1;
    body_end_idx = -1;
    for (size_t i = 0; i < data.size(); i++)
    {
        update_char(i);
    }
}

void http_buffer::add_chunk(std::deque<char> s)
{
    data.insert(data.end(), s.begin(), s.end());
    for (size_t i = data.size() - s.size(); (i < data.size()); i++)
    {
        update_char(i);
    }
}

bool http_buffer::header_available()
{
    return _was_header_end;
}

void http_buffer::update_char(int idx)
{
    if (!_was_body_end)
    {
        int beg = std::max(0, 1 + idx - (int)BODY_END.size());
        std::string cur(data.begin() + beg, data.begin() + idx + 1);
        if (cur == BODY_END)
        {
            _was_body_end = true;
            body_end_idx = idx;
        }
    }
    if (!_was_header_end)
    {
        for (int i = 0; i < 2; i++) {
            int beg = std::max(0, 1 + idx - (int)SEPARATORs[i].size());
            std::string cur(data.begin() + beg, data.begin() + idx + 1);
            if (cur == SEPARATORs[i])
            {
                _was_header_end = true;
                header_end_idx = idx;
            }
        }
    }
}

std::deque<char> http_buffer::substr(int from, int to)
{
    if (!((0 <= from) && (from <= to) && (to <= data.size())))
    {
        throw std::out_of_range("error in buffer::substr(from, to)");
    }
    std::deque<char> result(data.begin() + from, data.begin() + to);
    return result;
}

std::deque<char> http_buffer::extract_front_http(const http_parser &header)
{
    assert(_was_header_end);
    assert(false);
}

size_t http_buffer::size()
{
    return data.size();
}

std::string http_buffer::get_header()
{
    assert(_was_header_end);
    return std::string(data.begin(), data.begin() + header_end_idx + 1);
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
        char tmp[BUFF_SIZE];
        for (int j = 0; j < len; j++) tmp[j] = data[j];
        int _write = ::send(fd, tmp, len, MSG_NOSIGNAL);
        if (_write > 0)
        {
            data.erase(data.begin(), data.begin() + _write);
        } else if (_write < 0)
        {
            if ((errno == EINTR) || (errno == EPIPE)) continue;
            throw std::runtime_error("error in write():\n" + std::string(strerror(errno)));
        } else break;
    }
    return data.empty();
}
