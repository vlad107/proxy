#include "http_parser.h"

http_parser::http_parser()
{
    is_empty = true;
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
    std::cerr << "parsing of:\n" << header << "\n---" << std::endl;
    std::stringstream in(header);
    std::string line;
    getline(in, line); // type of request/response first line with version of HTTP
    header_items.clear();
    is_empty = false;
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
    std::cerr << "parsed" << std::endl;
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
        std::cerr << "Content-Length = " << content_len << std::endl;
        len = stoi(content_len);
    } catch (...)
    {
        len = 0;
    }
    return len;
}

std::string http_parser::get_host()
{
    return get_header_item("Host");
}
