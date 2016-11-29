#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H
#include <map>
#include <iostream>
#include <sstream>

class http_header
{
    std::map<std::string, std::string> items;
public:
    http_header();
    bool empty();
    std::string get_item(std::string name);
    void parse(std::string header);
    void clear();
};

#endif // HTTP_HEADER_H
