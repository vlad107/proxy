#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H
#include <map>
#include <iostream>
#include <assert.h>
#include <sstream>

#define RESPONSE_CHUNKED -1
#define RESPONSE_UNTIL_END -2
const std::string SEPARATORs[2] = {"\r\n\r\n", "\n\n"}; // TODO: \n\n unnecessarily
const std::string BODY_END = "0\r\n\r\n";

class http_parser
{
public:
    enum Version {HTTPS, HTTP10, HTTP11};
    enum Direction {REQUEST, RESPONSE};
    enum RequestType {GET, POST};
    http_parser();
    bool empty();
    void parse_header(std::string header, Direction dir);
    void clear();
    size_t get_content_length() const;
    Version get_ver() const;
    std::string get_host() const;
private:
    int response_code;
    Version extract_version(std::string);
    RequestType request_type;
    std::map<std::string, std::string> header_items;
    bool is_empty;
    std::string get_item(std::string name) const;
    bool _is_https;
    Version ver;
    Direction dir;
};

#endif // HTTP_PARSER_H
