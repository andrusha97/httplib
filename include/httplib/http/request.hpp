#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/http/headers.hpp>
#include <httplib/http/version.hpp>

#include <string>


HTTPLIB_OPEN_NAMESPACE


struct http_request_t {
    std::string method;
    std::string url;
    http_version_t version;
    http_headers_t headers;
};


std::ostream &operator<<(std::ostream &stream, const http_request_t &request);


HTTPLIB_CLOSE_NAMESPACE
