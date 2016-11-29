#include "buffer.h"

buffer::buffer()
{
    initialize();
}

void buffer::initialize()
{
    _was_header_end = false;
    sep_idx[0] = sep_idx[1] = 0;
    header_end = -1;
}

void buffer::add_chunk(std::string s)
{
    if (!_was_header_end)
    {
        for (size_t i = 0; i < s.size(); i++)
        {
            update_char(s[i], data.size() + i);
            if (_was_header_end) break;
        }
    }
    data.insert(data.end(), s.begin(), s.end());
}

bool buffer::was_header_end()
{
    return _was_header_end;
}

int buffer::get_header_end()
{
    return header_end;
}

void buffer::update_char(char c, int idx)
{
    for (int i = 0; i < 2; i++) {
        if ((sep_idx[i] < SEPARATORs[i].size()) && (c == SEPARATORs[i][sep_idx[i]]))
        {
            ++sep_idx[i];
        } else
        {
            sep_idx[i] = (c == SEPARATORs[i][0] ? 1 : 0);
        }
        if (sep_idx[i] == SEPARATORs[i].size())
        {
            _was_header_end = true;
            header_end = idx;
        }
    }
}

std::string buffer::substr(int from, int to)
{
    if (!((0 <= from) && (from <= to) && (to <= data.size())))
    {
        throw std::out_of_range("error in buffer::substr(from, to)");
    }
    std::string result(data.begin() + from, data.begin() + to);
    return result;
}

void buffer::remove_first_http_request(int body_len)
{
    data.erase(data.begin(), data.begin() + header_end + 1 + body_len);
    initialize();
    for (size_t i = 0; i < data.size(); i++) {
        update_char(data[i], i);
        if (_was_header_end) break;
    }
}

int buffer::size()
{
    return data.size();
}

std::string buffer::get_header()
{
    return substr(0, get_header_end() + 1);
}

std::string buffer::get_body(int body_len)
{
    return substr(get_header_end() + 1, get_header_end() + 1 + body_len);
}

