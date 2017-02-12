#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/error.hpp>
#include <httplib/asio/read_options.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/handler_alloc_hook.hpp>
#include <boost/asio/handler_continuation_hook.hpp>
#include <boost/asio/handler_invoke_hook.hpp>

#include <cstdlib>


HTTPLIB_OPEN_NAMESPACE


template<class BufferedReadStream>
chunked_body_reader<BufferedReadStream>::chunked_body_reader(BufferedReadStream &stream, read_options_t options) :
    m_stream(&stream),
    m_unconsumed_body(nullptr),
    m_unconsumed_body_size(0),
    m_last_parsed_part(0)
{
    set_options(options);
}


template<class BufferedReadStream>
void chunked_body_reader<BufferedReadStream>::set_options(read_options_t options) {
    m_options = options;
    m_parser.set_options(m_options.parsing);
}


template<class BufferedReadStream>
const http_headers_t &chunked_body_reader<BufferedReadStream>::trailer_headers() const {
    return m_parser.headers();
}


template<class BufferedReadStream>
boost::asio::io_service &chunked_body_reader<BufferedReadStream>::get_io_service() {
    return m_stream->stream().get_io_service();
}


template<class BufferedReadStream>
template<class MutableBuffers, class Handler>
typename boost::asio::async_result<
    typename boost::asio::handler_type<Handler, void(boost::system::error_code, std::size_t)>::type
>::type
chunked_body_reader<BufferedReadStream>::async_read_some(MutableBuffers buffers, Handler handler) {
    using handler_t = typename boost::asio::handler_type<Handler, void(boost::system::error_code, std::size_t)>::type;
    using op_t = async_read_some_op<MutableBuffers, handler_t>;

    handler_t concrete_handler = std::move(handler);
    boost::asio::async_result<handler_t> result(concrete_handler);
    op_t op(*this, buffers, std::move(concrete_handler));

    op.start();

    return result.get();
}


template<class BufferedReadStream>
template<class MutableBuffers>
std::size_t chunked_body_reader<BufferedReadStream>::read_some(MutableBuffers buffers, boost::system::error_code &ec) {
    if (boost::asio::buffer_size(buffers) == 0) {
        return 0;
    }

    while (true) {
        if (m_unconsumed_body_size > 0) {
            std::size_t transferred = boost::asio::buffer_copy(
                buffers,
                boost::asio::buffer(m_unconsumed_body, m_unconsumed_body_size)
            );

            m_unconsumed_body += transferred;
            m_unconsumed_body_size -= transferred;

            if (m_unconsumed_body_size == 0) {
                m_stream->buffer().consume(m_last_parsed_part);
            }

            return transferred;
        } else if (m_error) {
            ec = m_error;
            return 0;
        } else if (m_parser.done()) {
            m_error = make_error_code(httplib::reader_errc_t::eof);
            ec = m_error;
            return 0;
        }

        if (m_stream->buffer().size() == 0) {
            boost::system::error_code read_error;
            std::size_t transferred = m_stream->stream().read_some(
                m_stream->buffer().prepare(m_options.read_buffer_size),
                read_error
            );

            m_stream->buffer().commit(transferred);

            if (transferred == 0 && read_error) {
                ec = read_error;
                return 0;
            }
        }

        consume_read_buffer();
    }
}


template<class BufferedReadStream>
template<class MutableBuffers>
std::size_t chunked_body_reader<BufferedReadStream>::read_some(MutableBuffers buffers) {
    boost::system::error_code ec;
    std::size_t transferred = read_some(std::move(buffers), ec);

    if (ec) {
        throw boost::system::system_error(ec);
    }

    return transferred;
}


template<class BufferedReadStream>
void chunked_body_reader<BufferedReadStream>::consume_read_buffer() {
    auto buffers = m_stream->buffer().data();

    for (const auto &buf: buffers) {
        auto const_buffer = boost::asio::const_buffer(buf);

        while (boost::asio::buffer_size(const_buffer) > 0) {
            auto result = m_parser.parse(
                boost::asio::buffer_cast<const char *>(const_buffer),
                boost::asio::buffer_size(const_buffer)
            );

            m_last_parsed_part = result.parsed;

            // We will consume the data later, after copying it into the user's buffer.
            if (auto data = boost::get<chunked_body_parser_t::data_t>(&result.action)) {
                m_unconsumed_body = data->data;
                m_unconsumed_body_size = data->size;
                return;
            }

            const_buffer = boost::asio::const_buffer(
                boost::asio::buffer_cast<const char *>(const_buffer) + result.parsed,
                boost::asio::buffer_size(const_buffer) - result.parsed
            );

            m_stream->buffer().consume(result.parsed);

            if (auto error = boost::get<chunked_body_parser_t::error_t>(&result.action))
            {
                m_error = error->code;
                return;
            }
        }
    }
}


template<class BufferedReadStream>
template<class Buffers, class Handler>
struct chunked_body_reader<BufferedReadStream>::async_read_some_op {
    chunked_body_reader &reader;
    Buffers buffers;
    Handler handler;

    async_read_some_op(chunked_body_reader &reader, Buffers buffers, Handler handler) :
        reader(reader),
        buffers(buffers),
        handler(std::move(handler))
    { }

    void start() {
        if (reader.m_parser.done() || reader.m_error || reader.m_unconsumed_body_size > 0) {
            reader.m_stream->stream().get_io_service().post(std::move(*this));
            return;
        } else if (reader.m_stream->buffer().size() != 0) {
            reader.consume_read_buffer();

            if (reader.m_parser.done() || reader.m_error || reader.m_unconsumed_body_size > 0) {
                reader.m_stream->stream().get_io_service().post(std::move(*this));
                return;
            }
        }

        start_async_read();
    }

    void operator()() {
        if (reader.m_unconsumed_body_size > 0) {
            std::size_t transferred = boost::asio::buffer_copy(
                buffers,
                boost::asio::buffer(reader.m_unconsumed_body, reader.m_unconsumed_body_size)
            );

            reader.m_unconsumed_body += transferred;
            reader.m_unconsumed_body_size -= transferred;

            if (reader.m_unconsumed_body_size == 0) {
                reader.m_stream->buffer().consume(reader.m_last_parsed_part);
            }

            handler(boost::system::error_code(), transferred);
        } else if (reader.m_error) {
            handler(reader.m_error, 0);
        } else {
            reader.m_error = make_error_code(httplib::reader_errc_t::eof);
            handler(reader.m_error, 0);
        }
    }

    void operator()(boost::system::error_code ec, std::size_t transferred) {
        if (transferred > 0) {
            reader.m_stream->buffer().commit(transferred);
        }

        if (reader.m_stream->buffer().size() != 0) {
            reader.consume_read_buffer();

            if (reader.m_parser.done() || reader.m_error || reader.m_unconsumed_body_size > 0) {
                (*this)();
                return;
            }
        }

        if (ec) {
            if (!reader.m_error) {
                reader.m_error = ec;
            }

            handler(reader.m_error, 0);
            return;
        }

        start_async_read();
    }

    friend void *asio_handler_allocate(std::size_t size, async_read_some_op *context) {
        using boost::asio::asio_handler_allocate;
        return asio_handler_allocate(size, &context->handler);
    }

    friend void asio_handler_deallocate(void *pointer, std::size_t size, async_read_some_op *context) {
        using boost::asio::asio_handler_deallocate;
        asio_handler_deallocate(pointer, size, &context->handler);
    }

    friend bool asio_handler_is_continuation(async_read_some_op *context) {
        using boost::asio::asio_handler_is_continuation;
        return asio_handler_is_continuation(&context->handler);
    }

    template<class Callable>
    friend void asio_handler_invoke(Callable &function, async_read_some_op *context) {
        using boost::asio::asio_handler_invoke;
        asio_handler_invoke(function, &context->handler);
    }

    template<class Callable>
    friend void asio_handler_invoke(const Callable &function, async_read_some_op *context) {
        using boost::asio::asio_handler_invoke;
        asio_handler_invoke(function, &context->handler);
    }

private:
    void start_async_read() {
        reader.m_stream->stream().async_read_some(
            reader.m_stream->buffer().prepare(reader.m_options.read_buffer_size),
            std::move(*this)
        );
    }
};


HTTPLIB_CLOSE_NAMESPACE
