#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H
#include <map>
#include <iostream>
#include <assert.h>
#include <sstream>

#define REQUEST 1
#define RESPONSE 2

class http_parser
{
public:
    enum VERSION {HTTPS, HTTP10, HTTP11};
    http_parser();
    bool is_https();
    bool empty();
    void parse_header(std::string header);
    void clear();
    int get_content_len();
    VERSION get_ver();
    std::string get_host();
private:
    std::map<std::string, std::string> header_items;
    bool is_empty;
    std::string get_header_item(std::string name);
    bool _is_https;
    VERSION ver;
};

#endif // HTTP_PARSER_H
