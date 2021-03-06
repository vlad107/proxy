#include "http_buffer.h"

#include <algorithm>

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
    update_body_end(0);
    update_header_end(0);
}

bool http_buffer::body_available(const http_parser &header, bool closed)
{
    assert(_was_header_end);
    int code;
    size_t content_length = header.get_content_length(code);
    if (code == CHUNKED)
    {
        return _was_body_end;
    }
    if (code == UNTIL_DISCONNECT)
    {
        assert(header.get_dir() == http_parser::Direction::RESPONSE); // TODO: if client is HTTP/1.0 then it's possible for request
        return closed;
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
//    std::deque<char> result = data.extract_front(idx);
    std::deque<char> result(data.begin(), data.begin() + idx);
    data.erase(data.begin(), data.begin() + idx);
    initialize();
    return result;
}

void http_buffer::add_chunk(const std::deque<char> &s)
{
//    data.push_back(s);
    size_t old_size = data.size();
    data.insert(data.end(), s.begin(), s.end());
    update_body_end(old_size);
    update_header_end(old_size);
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

void http_buffer::update_body_end(size_t old_size)
{
    if (_was_body_end) return;
    int beg = old_size >= BODY_END.size() ? old_size - BODY_END.size() : 0;
    auto iter = std::search(data.begin() + beg, data.end(), BODY_END.begin(), BODY_END.end());
    if (iter != data.end())
    {
        _was_body_end = true;
        body_end_idx = (iter - data.begin()) + BODY_END.size() - 1;
    }
}

void http_buffer::update_header_end(size_t old_size)
{
    if (_was_header_end) return;
    int beg = old_size >= HEADER_END.size() ? old_size - HEADER_END.size() : 0;
    auto iter = std::search(data.begin() + beg, data.end(), HEADER_END.begin(), HEADER_END.end());
    if (iter != data.end())
    {
        _was_header_end = true;
        header_end_idx = (iter - data.begin()) + HEADER_END.size() - 1;
    }
}

//void http_buffer::update_char(size_t idx)
//{
//    if ((!_was_body_end) && (equals(idx, BODY_END)))
//    {
//        _was_body_end = true;
//        body_end_idx = idx;
//    }
//    if ((!_was_header_end) && (equals(idx, HEADER_END)))
//    {
//        _was_header_end = true;
//        header_end_idx = idx;
//    }
//}

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
//    bool _was = data.size() < MAX_BUFFER_SIZE;
    while (true)
    {
        const size_t BUFF_SIZE = 1024;
        size_t len = std::min(BUFF_SIZE, data.size());
        char tmp[BUFF_SIZE];
//        for (size_t j = 0; j < len; j++) tmp[j] = data[j];
        std::copy(data.begin(), data.begin() + len, tmp);
        int _write = ::send(fd, tmp, len, MSG_NOSIGNAL);
        if (_write > 0)
        {
            data.erase(data.begin(), data.begin() + _write);
        } else if (_write < 0)
        {
            if (EINTR == errno) continue;
            if ((EPIPE == errno) || (ECONNRESET == errno)) return data.size();
            throw std::runtime_error("error in write():\n" + std::string(strerror(errno)));
        } else break;
    }
//    if ((!_was) && (data.size() < MAX_BUFFER_SIZE))
//    {

//    }

    return data.empty();
}
