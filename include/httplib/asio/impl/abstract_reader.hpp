#pragma once

#include <httplib/detail/common.hpp>

#include <cstdlib>
#include <utility>


HTTPLIB_OPEN_NAMESPACE

namespace detail {

template<class Reader>
class reader_wrapper : public abstract_reader_t {
public:
    template<class... Args>
    reader_wrapper(Args &&... args) :
        m_backend(std::forward<Args>(args)...)
    { }

    boost::asio::io_service &get_io_service() override {
        return m_backend.get_io_service();
    }

    void async_read_some(erased_mutable_buffers_t buffers,
                         erased_handler<void(boost::system::error_code, std::size_t)> handler) override
    {
        m_backend.async_read_some(std::move(buffers), std::move(handler));
    }

    std::size_t read_some(erased_mutable_buffers_t buffers, boost::system::error_code &ec) override {
        return m_backend.read_some(std::move(buffers), ec);
    }

    std::size_t read_some(erased_mutable_buffers_t buffers) override {
        return m_backend.read_some(std::move(buffers));
    }

private:
    Reader m_backend;
};

} // namespace detail


template<class T, class... Args>
std::unique_ptr<abstract_reader_t> make_unique_reader(Args &&... args) {
    return std::make_unique<detail::reader_wrapper<T>>(std::forward<Args>(args)...);
}


HTTPLIB_CLOSE_NAMESPACE
