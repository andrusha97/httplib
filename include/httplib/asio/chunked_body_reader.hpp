#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/asio/read_options.hpp>
#include <httplib/parser/chunked_body_parser.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/io_service.hpp>

#include <cstdlib>


HTTPLIB_OPEN_NAMESPACE

template<class BufferedReadStream>
class chunked_body_reader {
public:
    explicit chunked_body_reader(BufferedReadStream &stream, read_options_t options = {});

    void set_options(read_options_t options);

    const http_headers_t &trailer_headers() const;

    boost::asio::io_service &get_io_service();

    template<class MutableBuffers, class Handler>
    typename boost::asio::async_result<
        typename boost::asio::handler_type<Handler, void(boost::system::error_code, std::size_t)>::type
    >::type
    async_read_some(MutableBuffers buffers, Handler handler);

    template<class MutableBuffers>
    std::size_t read_some(MutableBuffers buffers, boost::system::error_code &ec);

    template<class MutableBuffers>
    std::size_t read_some(MutableBuffers buffers);

private:
    void consume_read_buffer();

    template<class Buffers, class Handler>
    struct async_read_some_op;

private:
    BufferedReadStream *m_stream;
    read_options_t m_options;
    chunked_body_parser_t m_parser;
    boost::system::error_code m_error;
    const char *m_unconsumed_body;
    std::size_t m_unconsumed_body_size;
    std::size_t m_last_parsed_part;
};


HTTPLIB_CLOSE_NAMESPACE

#include <httplib/asio/impl/chunked_body_reader.hpp>
