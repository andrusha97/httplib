#include <httplib/response_builder.hpp>

#include <httplib/http/message_properties.hpp>

#include <boost/lexical_cast.hpp>


HTTPLIB_OPEN_NAMESPACE


http_response_builder_t::http_response_builder_t() :
    m_version({1, 1})
{ }


http_response_builder_t &http_response_builder_t::version(http_version_t version) {
    m_version = version;
    return *this;
}

http_response_builder_t &http_response_builder_t::add_header(const std::string &name, const std::string &value) {
    m_headers.add_header_values(name, {value});
    return *this;
}


http_response_builder_t &http_response_builder_t::keep_alive() {
    m_connection_status = connection_status_t::keep_alive;

    return *this;
}

http_response_builder_t &http_response_builder_t::connection_close() {
    m_connection_status = connection_status_t::close;

    return *this;
}

http_response_builder_t &http_response_builder_t::content_length(std::size_t length) {
    m_body_size = body_size_t{body_size_t::type_t::content_length, length};
    return *this;
}

http_response_builder_t &http_response_builder_t::chunked_encoding() {
    m_body_size = body_size_t{body_size_t::type_t::transfer_encoding, 0};
    return *this;
}


http_response_t http_response_builder_t::build(const status_code_t &status) const {
    return build(status.code(), status.description().to_string());
}

http_response_t http_response_builder_t::build(unsigned int code, const std::string &reason) const {
    http_response_t response;

    response.code = code;
    response.reason = reason;
    response.version = m_version;
    response.headers = m_headers;

    if (m_body_size) {
        switch (m_body_size->type) {
            case body_size_t::type_t::content_length: {
                response.headers.set_header("Content-Length", {std::to_string(m_body_size->content_length)});
            } break;
            case body_size_t::type_t::transfer_encoding: {
                response.headers.add_header_values("Transfer-Encoding", {"chunked"});
            } break;
            default: {
                assert(false);
            }
        }
    }

    if (m_connection_status) {
        switch (*m_connection_status) {
            case connection_status_t::close: {
                if (response.version >= http_version_t{1, 1}) {
                    response.headers.set_header("Connection", {"close"});
                }
            } break;
            case connection_status_t::keep_alive: {
                if (response.version <= http_version_t{1, 0}) {
                    response.headers.set_header("Connection", {"keep-alive"});
                }
            } break;
        }
    }

    return response;
}

http_version_t http_response_builder_t::version() const {
    return m_version;
}

http_headers_t http_response_builder_t::headers() const {
    return m_headers;
}

boost::optional<connection_status_t> http_response_builder_t::connection_status() const {
    return m_connection_status;
}

boost::optional<body_size_t> http_response_builder_t::body_size() const {
    return m_body_size;
}


result<http_response_builder_t, prepare_response_error_t>
prepare_response(const http_request_t &request) {
    using result_t = result<http_response_builder_t, prepare_response_error_t>;

    if (request.version.major < 1) {
        return make_error_result<result_t>(prepare_response_error_t::unsupported_version);
    }

    http_response_builder_t builder;

    if (auto status = connection_status(request)) {
        switch (*status) {
            case connection_status_t::keep_alive: {
                builder.keep_alive();
            } break;
            case connection_status_t::close: {
                builder.connection_close();
            } break;
        }
    } else {
        return make_error_result<result_t>(prepare_response_error_t::bad_message);
    }

    return result_t(std::move(builder));
}


status_code_t response_status_from_error(prepare_response_error_t error) {
    switch (error) {
        case prepare_response_error_t::bad_message:
            return STATUS_400_BAD_REQUEST;
        case prepare_response_error_t::unsupported_version:
            return STATUS_505_HTTP_VERSION_NOT_SUPPORTED;
    }

    // Passing an invalid enum value is definitely an internal server error.
    return STATUS_500_INTERNAL_SERVER_ERROR;
}


HTTPLIB_CLOSE_NAMESPACE
