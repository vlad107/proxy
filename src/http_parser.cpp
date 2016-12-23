#include "http_parser.h"

http_parser::http_parser()
    : is_empty(true),
      _is_https(false)
{
}

bool http_parser::is_https()
{
    return _is_https;
}

bool http_parser::empty()
{
    return is_empty;
}

std::string http_parser::get_header_item(std::string name)
{
    if (header_items.count(name) == 0)
    {
        throw std::runtime_error("item " + name + " not found in http-request");
    }
    return header_items[name];
}

void http_parser::parse_header(std::string header)
{
    std::stringstream in(header);
    std::string line;
    getline(in, line); // type of request/response first line with version of HTTP
    header_items.clear();
    is_empty = false;
    if (line.find("HTTPS") != std::string::npos)
    {
        _is_https = true;
    } else if (line.find("HTTP") != std::string::npos)
    {
        _is_https = false;
    } else assert(false);
    while (getline(in, line))
    {
        int sep = line.find(": ");
        if (sep != std::string::npos)
        {
            std::string name = line.substr(0, sep);
            std::string data = line.substr(sep + 2);
            header_items[name] = data;
        } else
        {
            std::cerr << "Warning: suspicious line in header of HTTP-request: " << line << std::endl;
        }
    }
}

void http_parser::clear()
{
    is_empty = true;
    header_items.clear();
}

int http_parser::get_content_len()
{
    int len;
    try
    {
        std::string content_len = get_header_item("Content-Length");
        len = stoi(content_len);
    } catch (...)
    {
        try
        {
            std::string type = get_header_item("Transfer-Encoding");
            if (type.find("chunked") != std::string::npos)
            {
                return -1;
            }
            return 0;
        } catch (...)
        {
            return 0;
        }
    }
    return len;
}

std::string http_parser::get_host()
{
    return get_header_item("Host");
}
