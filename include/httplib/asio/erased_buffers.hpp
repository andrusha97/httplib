#pragma once

#include <httplib/detail/common.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/container/small_vector.hpp>


HTTPLIB_OPEN_NAMESPACE

namespace detail {

struct const_buffer_t {
    const void *data;
    std::size_t size;

    operator boost::asio::const_buffer() const {
        return boost::asio::const_buffer(data, size);
    }
};

struct mutable_buffer_t {
    void *data;
    std::size_t size;

    operator boost::asio::mutable_buffer() const {
        return boost::asio::mutable_buffer(data, size);
    }
};

} // namespace detail

class erased_const_buffers_t {
public:
    using value_type = detail::const_buffer_t;
    using container_type = boost::container::small_vector<value_type, 3>;
    using const_iterator = container_type::const_iterator;

public:
    template<class ConstBuffers>
    erased_const_buffers_t(const ConstBuffers &buffers) {
        for (auto it = buffers.begin(); it != buffers.end(); ++it) {
            m_buffers.emplace_back(detail::const_buffer_t{
                boost::asio::buffer_cast<const void *>(*it),
                boost::asio::buffer_size(*it)
            });
        }
    }

    const_iterator begin() const {
        return m_buffers.begin();
    }

    const_iterator end() const {
        return m_buffers.end();
    }

private:
    container_type m_buffers;
};

class erased_mutable_buffers_t {
public:
    using value_type = detail::mutable_buffer_t;
    using container_type = boost::container::small_vector<value_type, 3>;
    using const_iterator = container_type::const_iterator;

public:
    template<class MutableBuffers>
    erased_mutable_buffers_t(const MutableBuffers &buffers) {
        for (auto it = buffers.begin(); it != buffers.end(); ++it) {
            m_buffers.emplace_back(detail::mutable_buffer_t{
                boost::asio::buffer_cast<void *>(*it),
                boost::asio::buffer_size(*it)
            });
        }
    }

    const_iterator begin() const {
        return m_buffers.begin();
    }

    const_iterator end() const {
        return m_buffers.end();
    }

private:
    container_type m_buffers;
};

HTTPLIB_CLOSE_NAMESPACE
