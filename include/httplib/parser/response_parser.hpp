#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/http/response.hpp>
#include <httplib/parser/parsing_options.hpp>

#include <boost/system/error_code.hpp>

#include <cstdlib>
#include <memory>


HTTPLIB_OPEN_NAMESPACE

class http_response_parser_t {
public:
    http_response_parser_t();
    http_response_parser_t(const http_response_parser_t &other);
    http_response_parser_t(http_response_parser_t &&other);

    ~http_response_parser_t();

    http_response_parser_t &operator=(const http_response_parser_t &other);
    http_response_parser_t &operator=(http_response_parser_t &&other);

    void set_options(http_parsing_options_t options);

    std::size_t parse(const char *data, std::size_t size);

    bool done() const;
    boost::system::error_code error() const;
    http_response_t &response();
    const http_response_t &response() const;

private:
    class implementation_t;

    std::unique_ptr<implementation_t> m_implementation;
};

HTTPLIB_CLOSE_NAMESPACE
