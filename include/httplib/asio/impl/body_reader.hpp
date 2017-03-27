#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/http/message_properties.hpp>
#include <httplib/parser/extension_list_parser.hpp>

#include <boost/variant.hpp>

#include <cassert>
#include <limits>


HTTPLIB_OPEN_NAMESPACE


template<class BufferedReadStream>
struct body_reader<BufferedReadStream>::get_io_service_visitor_t {
    boost::asio::io_service &operator()(boost::blank) const {
        assert(false);
    }

    boost::asio::io_service &operator()(eof_reader_t &stream) const {
        return stream.get_io_service();
    }

    boost::asio::io_service &operator()(bound_reader_t &stream) const {
        return stream.get_io_service();
    }

    boost::asio::io_service &operator()(chunked_reader_t &stream) const {
        return stream.get_io_service();
    }
};


template<class BufferedReadStream>
template<class MutableBuffers, class Handler>
struct body_reader<BufferedReadStream>::async_read_visitor {
    MutableBuffers &buffers;
    Handler &handler;

    typename boost::asio::async_result<
        typename boost::asio::handler_type<Handler, void(boost::system::error_code, std::size_t)>::type
    >::type
    operator()(boost::blank) const {
        assert(false);
    }

    typename boost::asio::async_result<
        typename boost::asio::handler_type<Handler, void(boost::system::error_code, std::size_t)>::type
    >::type
    operator()(eof_reader_t &stream) const {
        return stream.async_read_some(std::move(buffers), std::move(handler));
    }

    typename boost::asio::async_result<
        typename boost::asio::handler_type<Handler, void(boost::system::error_code, std::size_t)>::type
    >::type
    operator()(bound_reader_t &stream) const {
        return stream.async_read_some(std::move(buffers), std::move(handler));
    }

    typename boost::asio::async_result<
        typename boost::asio::handler_type<Handler, void(boost::system::error_code, std::size_t)>::type
    >::type
    operator()(chunked_reader_t &stream) const {
        return stream.async_read_some(std::move(buffers), std::move(handler));
    }
};


template<class BufferedReadStream>
template<class MutableBuffers>
struct body_reader<BufferedReadStream>::read_nothrow_visitor {
    MutableBuffers &buffers;
    boost::system::error_code &ec;

    std::size_t operator()(boost::blank) const {
        assert(false);
    }

    std::size_t operator()(eof_reader_t &stream) const {
        return stream.read_some(std::move(buffers), ec);
    }

    std::size_t operator()(bound_reader_t &stream) const {
        return stream.read_some(std::move(buffers), ec);
    }

    std::size_t operator()(chunked_reader_t &stream) const {
        return stream.read_some(std::move(buffers), ec);
    }
};


template<class BufferedReadStream>
template<class MutableBuffers>
struct body_reader<BufferedReadStream>::read_throw_visitor {
    MutableBuffers &buffers;

    std::size_t operator()(boost::blank) const {
        assert(false);
    }

    std::size_t operator()(eof_reader_t &stream) const {
        return stream.read_some(std::move(buffers));
    }

    std::size_t operator()(bound_reader_t &stream) const {
        return stream.read_some(std::move(buffers));
    }

    std::size_t operator()(chunked_reader_t &stream) const {
        return stream.read_some(std::move(buffers));
    }
};


namespace detail {

template<class BufferedReadStream>
result<body_reader<BufferedReadStream>, make_body_reader_error_t>
make_body_reader(body_size_t size, const http_headers_t &headers, BufferedReadStream &stream, read_options_t options) {
    using result_t = result<body_reader<BufferedReadStream>, make_body_reader_error_t>;

    switch (size.type) {
        case body_size_t::type_t::content_length: {
            return result_t(
                body_reader<BufferedReadStream>(bound_body_reader<BufferedReadStream>(stream, size.content_length, options))
            );
        } break;

        case body_size_t::type_t::transfer_encoding: {
            auto transfer_encoding = headers.get_header_values("Transfer-Encoding");

            if (!transfer_encoding || transfer_encoding->empty()) {
                return make_error_result<result_t>(make_body_reader_error_t::bad_message);
            }

            auto parsed = HTTPLIB_NAMESPACE::parse_extension_list(transfer_encoding->begin(), transfer_encoding->end());

            if (!parsed) {
                return make_error_result<result_t>(make_body_reader_error_t::bad_message);
            }

            if (parsed->extensions.size() != 1) {
                return make_error_result<result_t>(make_body_reader_error_t::unsupported_encoding);
            }

            if (*parsed->extensions.begin() != "chunked") {
                return make_error_result<result_t>(make_body_reader_error_t::bad_message);
            }

            return result_t(body_reader<BufferedReadStream>(chunked_body_reader<BufferedReadStream>(stream, options)));
        } break;

        case body_size_t::type_t::until_eof: {
            return result_t(body_reader<BufferedReadStream>(eof_body_reader<BufferedReadStream>(stream, options)));
        } break;
    }

    assert(false);
    return make_error_result<result_t>(make_body_reader_error_t::bad_message);
}

} // namespace detail


template<class BufferedReadStream>
result<body_reader<BufferedReadStream>, make_body_reader_error_t>
make_body_reader(const http_request_t &request, BufferedReadStream &stream, read_options_t options) {
    using result_t = result<body_reader<BufferedReadStream>, make_body_reader_error_t>;

    auto size = body_size(request);

    if (!size) {
        return make_error_result<result_t>(make_body_reader_error_t::bad_message);
    }

    return detail::make_body_reader(*size, request.headers, stream, options);
}


template<class BufferedReadStream>
result<body_reader<BufferedReadStream>, make_body_reader_error_t>
make_body_reader(const http_response_t &response, BufferedReadStream &stream, read_options_t options) {
    using result_t = result<body_reader<BufferedReadStream>, make_body_reader_error_t>;

    auto size = body_size(response);

    if (!size) {
        return make_error_result<result_t>(make_body_reader_error_t::bad_message);
    }

    return detail::make_body_reader(*size, response.headers, stream, options);
}


template<class BufferedReadStream>
result<body_reader<BufferedReadStream>, make_body_reader_error_t>
make_body_reader(const http_response_t &response,
                 const http_request_t &original_request,
                 BufferedReadStream &stream,
                 read_options_t options)
{
    using result_t = result<body_reader<BufferedReadStream>, make_body_reader_error_t>;

    auto size = body_size(response, original_request);

    if (!size) {
        return make_error_result<result_t>(make_body_reader_error_t::bad_message);
    }

    return detail::make_body_reader(*size, response.headers, stream, options);
}


HTTPLIB_CLOSE_NAMESPACE
