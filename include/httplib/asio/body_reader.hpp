#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/http/request.hpp>
#include <httplib/http/response.hpp>
#include <httplib/http/status_code.hpp>
#include <httplib/asio/bound_body_reader.hpp>
#include <httplib/asio/chunked_body_reader.hpp>
#include <httplib/result.hpp>

#include <boost/variant.hpp>


HTTPLIB_OPEN_NAMESPACE


template<class BufferedReadStream>
class body_reader {
    using bound_reader_t = bound_body_reader<BufferedReadStream>;
    using chunked_reader_t = chunked_body_reader<BufferedReadStream>;

public:
    body_reader() = default;

    body_reader(bound_reader_t reader) :
        m_reader(std::move(reader))
    { }

    body_reader(chunked_reader_t reader) :
        m_reader(std::move(reader))
    { }

    body_reader(body_reader &&other) :
        m_reader(std::move(other.m_reader))
    { }

    body_reader(const body_reader &other) = delete;

    body_reader &operator=(body_reader &&other) {
        m_reader = std::move(other.m_reader);
        return *this;
    }

    body_reader &operator=(const body_reader &other) = delete;

    boost::asio::io_service &get_io_service() {
        return boost::apply_visitor(get_io_service_visitor_t(), m_reader);
    }

    template<class MutableBuffers, class Handler>
    typename boost::asio::async_result<
        typename boost::asio::handler_type<Handler, void(boost::system::error_code, std::size_t)>::type
    >::type
    async_read_some(MutableBuffers buffers, Handler handler) {
        return boost::apply_visitor(async_read_visitor<MutableBuffers, Handler>{buffers, handler}, m_reader);
    }

    template<class MutableBuffers>
    std::size_t read_some(MutableBuffers buffers, boost::system::error_code &ec) {
        return boost::apply_visitor(read_nothrow_visitor<MutableBuffers>{buffers, ec}, m_reader);
    }

    template<class MutableBuffers>
    std::size_t read_some(MutableBuffers buffers) {
        return boost::apply_visitor(read_throw_visitor<MutableBuffers>{buffers}, m_reader);
    }

private:
    struct get_io_service_visitor_t;

    template<class MutableBuffers, class Handler>
    struct async_read_visitor;

    template<class MutableBuffers>
    struct read_nothrow_visitor;

    template<class MutableBuffers>
    struct read_throw_visitor;

private:
    using reader_t = boost::variant<boost::blank, bound_reader_t, chunked_reader_t>;

    reader_t m_reader;
};


enum class make_body_reader_error_t {
    bad_message,
    unsupported_encoding
};


template<class BufferedReadStream>
result<body_reader<BufferedReadStream>, make_body_reader_error_t>
make_body_reader(const http_request_t &request, BufferedReadStream &stream, read_options_t options = {});


template<class BufferedReadStream>
result<body_reader<BufferedReadStream>, make_body_reader_error_t>
make_body_reader(const http_response_t &response, BufferedReadStream &stream, read_options_t options = {});


template<class BufferedReadStream>
result<body_reader<BufferedReadStream>, make_body_reader_error_t>
make_body_reader(const http_response_t &response,
                 const http_request_t &original_request,
                 BufferedReadStream &stream,
                 read_options_t options = {});


inline status_code_t response_status_from_error(make_body_reader_error_t error) {
    switch (error) {
        case make_body_reader_error_t::bad_message:
            return STATUS_400_BAD_REQUEST;
        case make_body_reader_error_t::unsupported_encoding:
            return STATUS_501_NOT_IMPLEMENTED;
    }

    // Passing an invalid enum value is definitely an internal server error.
    return STATUS_500_INTERNAL_SERVER_ERROR;
}


HTTPLIB_CLOSE_NAMESPACE

#include <httplib/asio/impl/body_reader.hpp>
