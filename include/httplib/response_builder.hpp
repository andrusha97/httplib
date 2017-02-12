#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/http/message_properties.hpp>
#include <httplib/http/request.hpp>
#include <httplib/http/response.hpp>
#include <httplib/http/status_code.hpp>
#include <httplib/result.hpp>

#include <boost/optional.hpp>


HTTPLIB_OPEN_NAMESPACE

class http_response_builder_t {
public:
    http_response_builder_t();

    http_response_builder_t &version(http_version_t version);
    http_response_builder_t &add_header(const std::string &name, const std::string &value);
    http_response_builder_t &keep_alive() ;
    http_response_builder_t &connection_close();
    http_response_builder_t &content_length(std::size_t length);
    http_response_builder_t &chunked_encoding();

    http_response_t build(const status_code_t &status) const;
    http_response_t build(unsigned int code, const std::string &reason) const;

    http_version_t version() const;
    http_headers_t headers() const;
    boost::optional<connection_status_t> connection_status() const;
    boost::optional<body_size_t> body_size() const;

private:
    http_version_t m_version;
    http_headers_t m_headers;
    boost::optional<body_size_t> m_body_size;
    boost::optional<connection_status_t> m_connection_status;
};


enum prepare_response_error_t {
    bad_message,
    unsupported_version
};


result<http_response_builder_t, prepare_response_error_t>
prepare_response(const http_request_t &request);


status_code_t response_status_from_error(prepare_response_error_t error);

HTTPLIB_CLOSE_NAMESPACE
