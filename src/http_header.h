#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H
#include <map>
#include <iostream>
#include <sstream>

class http_header
{
    std::map<std::string, std::string> items;
    std::string get_item(std::string name);
public:
    http_header();
    bool empty();
    void parse(std::string header);
    void clear();
    int get_content_len();
    std::string get_host();
};

#endif // HTTP_HEADER_H
