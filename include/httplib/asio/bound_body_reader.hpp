#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/http/misc.hpp>
#include <httplib/asio/read_options.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/io_service.hpp>

#include <cstdlib>


HTTPLIB_OPEN_NAMESPACE


template<class BufferedReadStream>
class bound_body_reader {
public:
    bound_body_reader(BufferedReadStream &stream, content_length_int_t to_read, read_options_t options = {});

    void set_options(read_options_t options);

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
    template<class Buffers, class Handler>
    struct async_read_some_op;

private:
    BufferedReadStream *m_stream;
    read_options_t m_options;
    content_length_int_t m_to_read;
    content_length_int_t m_total_read;
};


HTTPLIB_CLOSE_NAMESPACE

#include <httplib/asio/impl/bound_body_reader.hpp>
