#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/http/response.hpp>
#include <httplib/asio/read_options.hpp>
#include <httplib/parser/response_parser.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/handler_alloc_hook.hpp>
#include <boost/asio/handler_continuation_hook.hpp>
#include <boost/asio/handler_invoke_hook.hpp>

#include <cstdlib>


HTTPLIB_OPEN_NAMESPACE

namespace detail {

template<class BufferedReadStream, class Handler>
struct async_read_response_op {
    BufferedReadStream &stream;
    read_options_t options;
    Handler handler;

    http_response_parser_t parser;


    async_read_response_op(BufferedReadStream &stream,
                           read_options_t options,
                           Handler handler) :
        stream(stream),
        options(options),
        handler(std::move(handler))
    {
        parser.set_options(options.parsing);
    }

    void start() {
        if (stream.buffer().size() != 0) {
            consume_buffer();

            if (parser.done()) {
                stream.stream().get_io_service().post(std::move(*this));
                return;
            }
        }

        start_async_read();
    }

    void operator()() {
        assert(parser.done());

        if (parser.error()) {
            handler(parser.error(), http_response_t());
        } else {
            handler(boost::system::error_code(), parser.response());
        }
    }

    void operator()(boost::system::error_code ec, std::size_t transferred) {
        if (transferred > 0) {
            stream.buffer().commit(transferred);
        }

        if (stream.buffer().size() != 0) {
            consume_buffer();

            if (parser.done()) {
                (*this)();
                return;
            }
        }

        if (ec) {
            handler(ec, http_response_t());
            return;
        }

        start_async_read();
    }

    friend void *asio_handler_allocate(std::size_t size, async_read_response_op *context) {
        using boost::asio::asio_handler_allocate;
        return asio_handler_allocate(size, &context->handler);
    }

    friend void asio_handler_deallocate(void *pointer, std::size_t size, async_read_response_op *context) {
        using boost::asio::asio_handler_deallocate;
        asio_handler_deallocate(pointer, size, &context->handler);
    }

    friend bool asio_handler_is_continuation(async_read_response_op *context) {
        using boost::asio::asio_handler_is_continuation;
        return asio_handler_is_continuation(&context->handler);
    }

    template<class Callable>
    friend void asio_handler_invoke(Callable &function, async_read_response_op *context) {
        using boost::asio::asio_handler_invoke;
        asio_handler_invoke(function, &context->handler);
    }

    template<class Callable>
    friend void asio_handler_invoke(const Callable &function, async_read_response_op *context) {
        using boost::asio::asio_handler_invoke;
        asio_handler_invoke(function, &context->handler);
    }

private:
    void consume_buffer() {
        if (stream.buffer().size() == 0) {
            return;
        }

        auto buffers = stream.buffer().data();

        for (const auto &buf: buffers) {
            auto const_buffer = boost::asio::const_buffer(buf);

            while (boost::asio::buffer_size(const_buffer) > 0) {
                size_t parsed = parser.parse(boost::asio::buffer_cast<const char *>(const_buffer),
                                             boost::asio::buffer_size(const_buffer));

                const_buffer = boost::asio::const_buffer(
                    boost::asio::buffer_cast<const char *>(const_buffer) + parsed,
                    boost::asio::buffer_size(const_buffer) - parsed
                );

                stream.buffer().consume(parsed);

                if (parser.done()) {
                    return;
                }
            }
        }
    }

    void start_async_read() {
        stream.stream().async_read_some(
            stream.buffer().prepare(options.read_buffer_size),
            std::move(*this)
        );
    }
};

} // namespace detail


template<class BufferedReadStream, class Handler>
typename boost::asio::async_result<
    typename boost::asio::handler_type<Handler, void(boost::system::error_code, const http_response_t&)>::type
>::type
async_read_response(BufferedReadStream &stream, read_options_t options, Handler handler) {
    using handler_t = typename boost::asio::handler_type<Handler, void(boost::system::error_code, const http_response_t&)>::type;
    using op_t = detail::async_read_response_op<BufferedReadStream, handler_t>;

    handler_t concrete_handler = std::move(handler);
    boost::asio::async_result<handler_t> result(concrete_handler);
    op_t op(stream, options, std::move(concrete_handler));

    op.start();

    return result.get();
}

template<class BufferedReadStream, class Handler>
typename boost::asio::async_result<
    typename boost::asio::handler_type<Handler, void(boost::system::error_code, const http_response_t&)>::type
>::type
async_read_response(BufferedReadStream &stream, Handler handler) {
    return async_read_response(stream, {}, std::move(handler));
}

template<class BufferedReadStream>
http_response_t read_response(BufferedReadStream &stream,
                              read_options_t options,
                              boost::system::error_code &ec)
{
    http_response_parser_t parser;
    parser.set_options(options.parsing);

    while (true) {
        auto buffers = stream.buffer().data();

        for (const auto &buffer: buffers) {
            if (boost::asio::buffer_size(buffer) == 0) {
                continue;
            }

            std::size_t parsed = parser.parse(boost::asio::buffer_cast<const char *>(buffer),
                                              boost::asio::buffer_size(buffer));

            stream.buffer().consume(parsed);

            if (parser.done()) {
                if (parser.error()) {
                    ec = parser.error();
                    return {};
                } else {
                    return parser.response();
                }
            }
        }

        boost::system::error_code read_error;

        std::size_t transferred = stream.stream().read_some(
            stream.buffer().prepare(options.read_buffer_size),
            read_error
        );

        stream.buffer().commit(transferred);

        if (transferred == 0 && read_error) {
            ec = read_error;
            return {};
        }
    }
};

template<class BufferedReadStream>
http_response_t read_response(BufferedReadStream &stream, boost::system::error_code &ec) {
    return read_response(stream, {}, ec);
}

template<class BufferedReadStream>
http_response_t read_response(BufferedReadStream &stream, read_options_t options) {
    boost::system::error_code ec;
    http_response_t result = read_response(stream, std::move(options));

    if (ec) {
        throw boost::system::system_error(ec);
    }

    return result;
}

template<class BufferedReadStream>
http_response_t read_response(BufferedReadStream &stream) {
    return read_response(stream, {});
}

HTTPLIB_CLOSE_NAMESPACE
