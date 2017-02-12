#pragma once

#include <httplib/detail/common.hpp>

#include <boost/asio/streambuf.hpp>

#include <cstdlib>
#include <type_traits>


HTTPLIB_OPEN_NAMESPACE


template<class ReadStream, class Buffer>
class buffered_read_stream {
    using stream_type = std::remove_reference_t<ReadStream>;
    using buffer_type = std::remove_reference_t<Buffer>;

public:
    template<class StreamInit, class BufferInit>
    buffered_read_stream(StreamInit &&stream, BufferInit &&buffer) :
        m_stream(std::forward<StreamInit>(stream)),
        m_buffer(std::forward<BufferInit>(buffer))
    { }

    stream_type &stream() {
        return m_stream;
    }

    const stream_type &stream() const {
        return m_stream;
    }

    buffer_type &buffer() {
        return m_buffer;
    }

    const buffer_type &buffer() const {
        return m_buffer;
    }

private:
    ReadStream m_stream;
    Buffer m_buffer;
};


HTTPLIB_CLOSE_NAMESPACE
