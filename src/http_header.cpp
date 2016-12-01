#include "http_header.h"

http_header::http_header()
{

}

bool http_header::empty()
{
    return items.empty();
}

std::string http_header::get_item(std::string name)
{
    if (items.count(name) == 0)
    {
        throw std::runtime_error("item " + name + " not found in http-request");
    }
    return items[name];
}

void http_header::parse(std::string header)
{
    std::cerr << "parsing of:\n" << header << "\n---" << std::endl;
    std::stringstream in(header);
    std::string line;
    getline(in, line); // type of request/response first line with version of HTTP
    while (getline(in, line))
    {
        int sep = line.find(": ");
        if (sep != std::string::npos)
        {
            std::string name = line.substr(0, sep);
            std::string data = line.substr(sep + 2);
            items[name] = data;
        } else
        {
            std::cerr << "Warning: suspicious line in header of HTTP-request: " << line << std::endl;
        }
    }
    std::cerr << "parsed" << std::endl;
}

void http_header::clear()
{
    items.clear();
}

int http_header::get_content_len()
{
    int len;
    try
    {
        len = stoi(get_item("Content-Length"));
    } catch (...)
    {
        len = 0;
    }
    return len;
}

std::string http_header::get_host()
{
    return get_item("Host");
}
