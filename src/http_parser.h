#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H
#include <map>
#include <iostream>
#include <assert.h>
#include <sstream>

#define RESPONSE_CHUNKED -1
#define RESPONSE_UNTIL_END -2

class http_parser
{
public:
    enum VERSION {HTTPS, HTTP10, HTTP11};
    enum DIRECTION {REQUEST, RESPONSE};
    enum REQUEST_TYPE {GET, POST};
    http_parser();
    bool empty();
    void parse_header(std::string header, DIRECTION dir);
    void clear();
    size_t get_content_length() const;
    VERSION get_ver() const;
    std::string get_host() const;
private:
    int response_code;
    VERSION extract_version(std::string);
    REQUEST_TYPE request_type;
    std::map<std::string, std::string> header_items;
    bool is_empty;
    std::string get_item(std::string name) const;
    bool _is_https;
    VERSION ver;
    DIRECTION dir;
};

#endif // HTTP_PARSER_H
