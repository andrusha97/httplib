#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/http/headers.hpp>
#include <httplib/parser/parsing_options.hpp>

#include <boost/system/error_code.hpp>
#include <boost/variant.hpp>

#include <cstdlib>
#include <memory>


HTTPLIB_OPEN_NAMESPACE

class chunked_body_parser_t {
public:
    struct none_t { };

    struct data_t {
        const char *data;
        std::size_t size;
    };

    struct error_t {
        boost::system::error_code code;
    };

    using action_t = boost::variant<none_t, data_t, error_t>;

    struct result_t {
        std::size_t parsed;
        action_t action;
    };

public:
    chunked_body_parser_t();
    chunked_body_parser_t(const chunked_body_parser_t &other);
    chunked_body_parser_t(chunked_body_parser_t &&other);

    ~chunked_body_parser_t();

    chunked_body_parser_t &operator=(const chunked_body_parser_t &other);
    chunked_body_parser_t &operator=(chunked_body_parser_t &&other);

    void set_options(http_parsing_options_t options);

    result_t parse(const char *data, std::size_t size);

    bool done() const;
    http_headers_t &headers();
    const http_headers_t &headers() const;

private:
    class implementation_t;

    std::unique_ptr<implementation_t> m_implementation;
};

HTTPLIB_CLOSE_NAMESPACE
