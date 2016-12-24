#include "http_parser.h"

http_parser::http_parser()
    : _empty(true),
      response_code(0)
{
}

void http_parser::clear()
{
    _empty = true;
    header_items.clear();
}

bool http_parser::empty() const
{
    return _empty;
}

std::string http_parser::get_item(std::string name) const
{
    if (header_items.count(name) == 0)
    {
        throw std::runtime_error("item " + name + " not found in http-request");
    }
    return header_items.find(name)->second;
}

http_parser::Version http_parser::get_ver() const
{
    return ver;
}

http_parser::Direction http_parser::get_dir() const
{
    return dir;
}

http_parser::Version http_parser::extract_version(std::string line)
{
    std::cerr << "extract version from " << line << std::endl;
    if (line.find("HTTPS") != std::string::npos)
    {
        return Version::HTTPS;
    }
    if (line.find("HTTP/1.0") != std::string::npos)
    {
        return Version::HTTP10;
    }
    if (line.find("HTTP/1.1") != std::string::npos)
    {
        return Version::HTTP11;
    }
    assert(false); // todo: ver = other should be here
}

void http_parser::parse_header(std::string header, http_parser::Direction dir)
{
    this->dir = dir;
    std::stringstream in(header);
    std::string line;
    getline(in, line); // type of request/response first line with version of HTTP
    header_items.clear();
    _empty = false;
    ver = extract_version(line);
    if (dir == Direction::RESPONSE)
    {
        line.erase(0, line.find(" "));
        response_code = std::stoi(line);
        std::cerr << "RESPONSE CODE = " << response_code << std::endl;
    }
    if (dir == Direction::REQUEST)
    {
        if (line.find("GET") != std::string::npos)
        {
            request_type = RequestType::GET;
        } else if (line.find("POST") != std::string::npos)
        {
            request_type = RequestType::POST;
        } else assert(false);
    }
    while (getline(in, line))
    {
        size_t sep = line.find(": ");
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

size_t http_parser::get_content_length() const
{
    if ((dir == Direction::RESPONSE) &&
        (((100 <= response_code) && (response_code <= 199)) || (response_code == 204) || (response_code == 304)))
    {
        return 0;
    }
    if ((dir == Direction::REQUEST) && (request_type == RequestType::GET))
    {
        return 0; // TODO: not always, but very often
    }
    try
    {
        std::string connection = get_item("Connection");
        if (connection.find("close") != std::string::npos)
        {
            return UNTIL_DISCONNECT;
        }
        throw std::runtime_error("connection != close");
    } catch (...)
    {
        try
        {
            std::string encoding = get_item("Transfer-Encoding");
            if (encoding.find("identity") == std::string::npos)
            {
                return CHUNKED;
            }
            assert(false); // TODO: have not seen what to do in this case in RFC
        } catch (...)
        {
            try
            {
                std::string length = get_item("Content-Length");
                std::cerr << "Content-Length = " << length << std::endl;
                return std::stoi(length);
            } catch (...)
            {
                assert(false);
            }
        }
    }
}


std::string http_parser::get_host() const
{
    return get_item("Host");
}
