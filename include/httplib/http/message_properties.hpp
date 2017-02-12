#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/http/misc.hpp>
#include <httplib/http/request.hpp>
#include <httplib/http/response.hpp>

#include <boost/optional.hpp>


HTTPLIB_OPEN_NAMESPACE

struct body_size_t {
    enum class type_t {
        content_length,
        transfer_encoding,
        until_eof
    };

    type_t type;
    content_length_int_t content_length;
};

boost::optional<body_size_t> body_size(const http_request_t &request);
boost::optional<body_size_t> body_size(const http_response_t &response);
boost::optional<body_size_t> body_size(const http_response_t &response, const http_request_t &original_request);


enum class connection_status_t {
    keep_alive,
    close
};

boost::optional<connection_status_t> connection_status(const http_request_t &request);
boost::optional<connection_status_t> connection_status(const http_response_t &response);


HTTPLIB_CLOSE_NAMESPACE
