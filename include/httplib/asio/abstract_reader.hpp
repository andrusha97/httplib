#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/asio/erased_buffers.hpp>
#include <httplib/asio/erased_handler.hpp>

#include <boost/asio/io_service.hpp>

#include <cstdlib>
#include <utility>


HTTPLIB_OPEN_NAMESPACE


class abstract_reader_t {
public:
    virtual ~abstract_reader_t() { }

    virtual boost::asio::io_service &get_io_service() = 0;
    virtual void async_read_some(erased_mutable_buffers_t, erased_handler<void(boost::system::error_code, std::size_t)>) = 0;
    virtual std::size_t read_some(erased_mutable_buffers_t, boost::system::error_code &) = 0;
    virtual std::size_t read_some(erased_mutable_buffers_t) = 0;
};


template<class T, class... Args>
std::unique_ptr<abstract_reader_t> make_unique_reader(Args &&... args);


HTTPLIB_CLOSE_NAMESPACE

#include <httplib/asio/impl/abstract_reader.hpp>
