#pragma once

#include <httplib/detail/common.hpp>

#include <cstdlib>


HTTPLIB_OPEN_NAMESPACE


struct http_parsing_options_t {
    std::size_t max_url_size = 8 * 1024;
    std::size_t max_reason_size = 8 * 1024;
    std::size_t max_header_size = 8 * 1024;
    std::size_t max_headers_number = 256;
};


HTTPLIB_CLOSE_NAMESPACE
