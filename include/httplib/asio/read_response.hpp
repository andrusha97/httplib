#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/http/response.hpp>
#include <httplib/asio/read_options.hpp>

#include <boost/asio/async_result.hpp>


HTTPLIB_OPEN_NAMESPACE


template<class BufferedReadStream, class Handler>
typename boost::asio::async_result<
    typename boost::asio::handler_type<Handler, void(boost::system::error_code, const http_response_t&)>::type
>::type
async_read_response(BufferedReadStream &stream, read_options_t options, Handler handler);


template<class BufferedReadStream, class Handler>
typename boost::asio::async_result<
    typename boost::asio::handler_type<Handler, void(boost::system::error_code, const http_response_t&)>::type
>::type
async_read_response(BufferedReadStream &stream, Handler handler);


template<class BufferedReadStream>
http_response_t read_response(BufferedReadStream &stream,
                              read_options_t options,
                              boost::system::error_code &ec);


template<class BufferedReadStream>
http_response_t read_response(BufferedReadStream &stream, boost::system::error_code &ec);


template<class BufferedReadStream>
http_response_t read_response(BufferedReadStream &stream, read_options_t options);


template<class BufferedReadStream>
http_response_t read_response(BufferedReadStream &stream);


HTTPLIB_CLOSE_NAMESPACE

#include <httplib/asio/impl/read_response.hpp>
