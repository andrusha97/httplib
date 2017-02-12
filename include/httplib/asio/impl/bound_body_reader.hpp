#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/error.hpp>
#include <httplib/http/misc.hpp>
#include <httplib/asio/read_options.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/handler_alloc_hook.hpp>
#include <boost/asio/handler_continuation_hook.hpp>
#include <boost/asio/handler_invoke_hook.hpp>

#include <cstdlib>


HTTPLIB_OPEN_NAMESPACE


template<class BufferedReadStream>
bound_body_reader<BufferedReadStream>::bound_body_reader(
    BufferedReadStream &stream,
    content_length_int_t to_read,
    read_options_t options
) :
    m_stream(&stream),
    m_to_read(to_read),
    m_total_read(0)
{
    set_options(options);
}


template<class BufferedReadStream>
void bound_body_reader<BufferedReadStream>::set_options(read_options_t options) {
    m_options = options;
}


template<class BufferedReadStream>
boost::asio::io_service &bound_body_reader<BufferedReadStream>::get_io_service() {
    return m_stream->stream().get_io_service();
}


// Pizdets.
template<class BufferedReadStream>
template<class MutableBuffers, class Handler>
typename boost::asio::async_result<
    typename boost::asio::handler_type<Handler, void(boost::system::error_code, std::size_t)>::type
>::type
bound_body_reader<BufferedReadStream>::async_read_some(MutableBuffers buffers, Handler handler) {
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
std::size_t bound_body_reader<BufferedReadStream>::read_some(MutableBuffers buffers, boost::system::error_code &ec) {
    if (boost::asio::buffer_size(buffers) == 0) {
        return 0;
    } else if (m_total_read >= m_to_read) {
        ec = make_error_code(httplib::reader_errc_t::eof);
        return 0;
    } else {
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

        std::size_t transferred = boost::asio::buffer_copy(
            buffers,
            m_stream->buffer().data(),
            m_to_read - m_total_read
        );

        m_stream->buffer().consume(transferred);
        m_total_read += transferred;

        return transferred;
    }
}


template<class BufferedReadStream>
template<class MutableBuffers>
std::size_t bound_body_reader<BufferedReadStream>::read_some(MutableBuffers buffers) {
    boost::system::error_code ec;
    std::size_t transferred = read_some(std::move(buffers), ec);

    if (ec) {
        throw boost::system::system_error(ec);
    }

    return transferred;
}


template<class BufferedReadStream>
template<class Buffers, class Handler>
struct bound_body_reader<BufferedReadStream>::async_read_some_op {
    bound_body_reader &reader;
    Buffers buffers;
    Handler handler;

    async_read_some_op(bound_body_reader &reader, Buffers buffers, Handler handler) :
        reader(reader),
        buffers(buffers),
        handler(std::move(handler))
    { }

    void start() {
        if (reader.m_total_read >= reader.m_to_read ||
            reader.m_stream->buffer().size() != 0 ||
            boost::asio::buffer_size(buffers) == 0)
        {
            reader.m_stream->stream().get_io_service().post(std::move(*this));
        } else {
            reader.m_stream->stream().async_read_some(
                reader.m_stream->buffer().prepare(reader.m_options.read_buffer_size),
                std::move(*this)
            );
        }
    }

    void operator()() {
        if (reader.m_total_read >= reader.m_to_read) {
            handler(make_error_code(httplib::reader_errc_t::eof), 0);
        } else {
            std::size_t transferred = boost::asio::buffer_copy(
                buffers,
                reader.m_stream->buffer().data(),
                reader.m_to_read - reader.m_total_read
            );

            reader.m_stream->buffer().consume(transferred);
            reader.m_total_read += transferred;
            handler(boost::system::error_code(), transferred);
        }
    }

    void operator()(boost::system::error_code ec, std::size_t transferred) {
        if (transferred > 0) {
            reader.m_stream->buffer().commit(transferred);
        }

        if (reader.m_stream->buffer().size() != 0) {
            (*this)();
            return;
        }

        if (ec) {
            handler(ec, 0);
            return;
        }

        handler(make_error_code(boost::asio::error::try_again), 0);
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
};


HTTPLIB_CLOSE_NAMESPACE
