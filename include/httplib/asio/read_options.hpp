#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/parser/parsing_options.hpp>

#include <cstdlib>


HTTPLIB_OPEN_NAMESPACE

struct read_options_t {
    http_parsing_options_t parsing;
    std::size_t read_buffer_size = 4 * 1024;
};

HTTPLIB_CLOSE_NAMESPACE
