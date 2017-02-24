#include <httplib/http/message_properties.hpp>

#include <httplib/parser/token_list_parser.hpp>

#include <boost/lexical_cast.hpp>


HTTPLIB_OPEN_NAMESPACE


boost::optional<body_size_t> body_size(const http_request_t &request) {
    if (request.version >= http_version_t{1, 1}) {
        if (auto transfer_encoding = request.headers.get_header_values("Transfer-Encoding")) {
            if (!transfer_encoding->empty()) {
                return body_size_t{body_size_t::type_t::transfer_encoding, 0};
            }
        }
    }

    if (auto content_length = request.headers.get_header_values("Content-Length")) {
        if (content_length->size() != 1) {
            return boost::none;
        }

        content_length_int_t parsed_length = 0;

        if (!boost::conversion::try_lexical_convert((*content_length)[0].data(), (*content_length)[0].size(), parsed_length)) {
            return boost::none;
        }

        return body_size_t{body_size_t::type_t::content_length, parsed_length};
    } else {
        return body_size_t{body_size_t::type_t::content_length, 0};
    }
}


boost::optional<body_size_t> body_size(const http_response_t &response) {
    if ((response.code >= 100 && response.code < 200) || response.code == 204 || response.code == 304) {
        return body_size_t{body_size_t::type_t::content_length, 0};
    }

    if (response.version >= http_version_t{1, 1}) {
        if (auto transfer_encoding = response.headers.get_header_values("Transfer-Encoding")) {
            if (!transfer_encoding->empty()) {
                return body_size_t{body_size_t::type_t::transfer_encoding, 0};
            }
        }
    }

    if (auto content_length = response.headers.get_header_values("Content-Length")) {
        if (content_length->size() != 1) {
            return boost::none;
        }

        content_length_int_t parsed_length = 0;

        if (!boost::conversion::try_lexical_convert((*content_length)[0].data(), (*content_length)[0].size(), parsed_length)) {
            return boost::none;
        }

        return body_size_t{body_size_t::type_t::content_length, parsed_length};
    } else {
        return body_size_t{body_size_t::type_t::until_eof, 0};
    }
}


boost::optional<body_size_t> body_size(const http_response_t &response,
                                       const http_request_t &original_request)
{
    if ((response.code >= 100 && response.code < 200) || response.code == 204 || response.code == 304) {
        return body_size_t{body_size_t::type_t::content_length, 0};
    }

    if (original_request.method == "HEAD") {
        return body_size_t{body_size_t::type_t::content_length, 0};
    }

    if (original_request.method == "CONNECT" && response.code >= 200 && response.code < 300) {
        return body_size_t{body_size_t::type_t::content_length, 0};
    }

    if (response.version >= http_version_t{1, 1}) {
        if (auto transfer_encoding = response.headers.get_header_values("Transfer-Encoding")) {
            if (!transfer_encoding->empty()) {
                return body_size_t{body_size_t::type_t::transfer_encoding, 0};
            }
        }
    }

    if (auto content_length = response.headers.get_header_values("Content-Length")) {
        if (content_length->size() != 1) {
            return boost::none;
        }

        content_length_int_t parsed_length = 0;

        if (!boost::conversion::try_lexical_convert((*content_length)[0].data(), (*content_length)[0].size(), parsed_length)) {
            return boost::none;
        }

        return body_size_t{body_size_t::type_t::content_length, parsed_length};
    } else {
        return body_size_t{body_size_t::type_t::until_eof, 0};
    }
}


namespace {

template<class MessageHead>
boost::optional<connection_status_t> connection_status_impl(const MessageHead &message) {
    bool has_close = false;
    bool has_keep_alive = false;

    if (auto connection = message.headers.get_header_values("Connection")) {
        auto tokens = parse_token_list(connection->begin(), connection->end());

        if (!tokens) {
            return boost::none;
        }

        has_close = tokens->has("close");
        has_keep_alive = tokens->has("keep-alive");
    }

    if (has_close) {
        return connection_status_t::close;
    } else if (message.version >= http_version_t{1, 1}) {
        return connection_status_t::keep_alive;
    } else if (message.version == http_version_t{1, 0} && has_keep_alive) {
        return connection_status_t::keep_alive;
    } else {
        return connection_status_t::close;
    }
}

} // namespace


boost::optional<connection_status_t> connection_status(const http_request_t &request) {
    return connection_status_impl(request);
}

boost::optional<connection_status_t> connection_status(const http_response_t &response) {
    return connection_status_impl(response);
}


HTTPLIB_CLOSE_NAMESPACE
