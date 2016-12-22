#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H
#include <map>
#include <iostream>
#include <sstream>

#define REQUEST 1
#define RESPONSE 2

class http_parser
{
    std::map<std::string, std::string> header_items;
    bool is_empty;
    std::string get_header_item(std::string name);
public:
    http_parser();
    bool empty();
    void parse_header(std::string header);
    void clear();
    int get_content_len();
    std::string get_host();
};

#endif // HTTP_PARSER_H
