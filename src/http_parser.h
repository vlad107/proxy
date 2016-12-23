#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H
#include <map>
#include <iostream>
#include <assert.h>
#include <sstream>

#define CHUNKED -1
#define UNTIL_DISCONNECT -2

const std::string HEADER_END = "\r\n\r\n";
const std::string BODY_END = "0\r\n\r\n";

class http_parser
{
public:
    enum Version {HTTPS, HTTP10, HTTP11};
    enum Direction {REQUEST, RESPONSE};
    enum RequestType {GET, POST};
    http_parser();

    void parse_header(std::string header, Direction dir);
    void clear();

    bool empty() const;
    size_t get_content_length() const;
    Version get_ver() const;
    Direction get_dir() const;
    std::string get_host() const;
private:
    bool _empty;
    int response_code;
    RequestType request_type;
    Version ver;
    Direction dir;
    std::map<std::string, std::string> header_items;

    Version extract_version(std::string);
    std::string get_item(std::string name) const;
};

#endif // HTTP_PARSER_H
