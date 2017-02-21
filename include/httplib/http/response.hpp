#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/http/headers.hpp>
#include <httplib/http/version.hpp>

#include <string>


HTTPLIB_OPEN_NAMESPACE


struct http_response_t {
    unsigned int code = 0;
    std::string reason;
    http_version_t version;
    http_headers_t headers;
};


std::ostream &operator<<(std::ostream &stream, const http_response_t &response);


HTTPLIB_CLOSE_NAMESPACE
