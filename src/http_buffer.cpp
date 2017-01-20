#include "http_buffer.h"

http_buffer::http_buffer()
{
    initialize();
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

bool http_buffer::body_available(const http_parser &header, bool started)
{
    assert(_was_header_end);
    int code;
    size_t content_length = header.get_content_length(code);
    if (CHUNKED == code)
    {
        return _was_body_end;
    }
    if (UNTIL_DISCONNECT == code)
    {
        assert(header.get_dir() == http_parser::Direction::RESPONSE); // TODO: if client is HTTP/1.0 then it's possible for request
        return !started;
    }
    return size() - header_end_idx >= content_length;
}

std::deque<char> http_buffer::extract_front_http(const http_parser &header)
{
    assert(_was_header_end);
    int code;
    size_t content_length = header.get_content_length(code);
    size_t idx;
    if (CHUNKED == code)
    {
        idx = body_end_idx + 1;
    } else if (UNTIL_DISCONNECT == code)
    {
        idx = data.size();
    } else
    {
        idx = header_end_idx + content_length + 1;
    }
    std::deque<char> result(data.begin(), data.begin() + idx);
    data.erase(data.begin(), data.begin() + idx);
    initialize();
    return result;
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

bool http_buffer::equals(size_t idx, const std::string s)
{
    assert(idx < data.size());
    size_t beg = 1 + idx >= s.size() ? (1 + idx - s.size()) : 0;
    std::string cur(data.begin() + beg, data.begin() + idx + 1);
    return cur == s;
}

void http_buffer::update_char(size_t idx)
{
    if ((!_was_body_end) && (equals(idx, BODY_END)))
    {
        _was_body_end = true;
        body_end_idx = idx;
    }
    if ((!_was_header_end) && (equals(idx, HEADER_END)))
    {
        _was_header_end = true;
        header_end_idx = idx;
    }
}

std::deque<char> http_buffer::substr(size_t from, size_t to)
{
    if (!((from <= to) && (to <= data.size())))
    {
        throw std::out_of_range("error in buffer::substr(from, to)");
    }
    std::deque<char> result(data.begin() + from, data.begin() + to);
    return result;
}

size_t http_buffer::size()
{
    return data.size();
}

std::string http_buffer::get_header()
{
    assert(_was_header_end);
    std::string result(data.begin(), data.begin() + header_end_idx + 1);
    return result;
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
        for (size_t j = 0; j < len; j++) tmp[j] = data[j];
        int _write = ::send(fd, tmp, len, MSG_NOSIGNAL);
        if (_write > 0)
        {
            data.erase(data.begin(), data.begin() + _write);
        } else if (_write < 0)
        {
            if (EINTR == errno) continue;
            if (EPIPE == errno) return data.size();
            throw std::runtime_error("error in write():\n" + std::string(strerror(errno)));
        } else break;
    }
    return data.empty();
}
