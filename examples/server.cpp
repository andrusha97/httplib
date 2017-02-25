#include <httplib/asio/body_reader.hpp>
#include <httplib/asio/buffered_read_stream.hpp>
#include <httplib/asio/read_request.hpp>

#include <httplib/http/message_properties.hpp>
#include <httplib/http/url.hpp>

#include <httplib/response_builder.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/lexical_cast.hpp>

#include <array>
#include <iostream>
#include <string>
#include <vector>


inline std::string escape_string(const char *data, size_t size) {
    std::string result;

    const char *hex_digits = "0123456789abcdef";

    for (auto it = data; it < data + size; ++it) {
        if (*it == '\n') {
            result.append("\\n");
        } else if (*it == '\r') {
            result.append("\\r");
        } else if (*it == '\t') {
            result.append("\\t");
        } else if (*it == '\\') {
            result.append("\\\\");
        } else if (std::isprint(*it)) {
            result.push_back(*it);
        } else {
            result.append("\\x");

            const unsigned char n = *it;
            result.push_back(hex_digits[n / 16]);
            result.push_back(hex_digits[n % 16]);
        }
    }

    return result;
}


class connection_handler_t : public std::enable_shared_from_this<connection_handler_t> {
public:
    connection_handler_t(boost::asio::io_service &io_service) :
        m_socket(io_service),
        m_bufstream(m_socket, m_buffer)
    { }

    void start() {
        httplib::async_read_request(m_bufstream, {}, [self = shared_from_this()](auto ec, auto request) {
            self->handle_request(ec, request);
        });
    }

private:
    void handle_request(boost::system::error_code ec, httplib::http_request_t req) {
        if (ec) {
            std::cerr << "Failed to read request: " << ec.message() << std::endl;
            return;
        }

        std::cerr << "Received a request:" << std::endl
                  << req << std::endl;

        if (auto raw_url = httplib::parse_url(req.url)) {
            auto url = httplib::normalize_url(*raw_url);

            std::cerr << "Parsed url:" << std::endl;
            std::cerr << "Schema: " << url.schema.value_or("none") << std::endl;
            std::cerr << "Host: " << url.host.value_or("none") << std::endl;

            if (url.port) {
                std::cerr << "Port: " << *url.port << std::endl;
            } else {
                std::cerr << "Port: none" << std::endl;
            }

            std::cerr << "Path: " << url.path << std::endl;
            std::cerr << "Query: " << url.query.value_or("none") << std::endl;
            std::cerr << "Fragment: " << url.fragment.value_or("none") << std::endl;


            if (auto query = httplib::parse_query(url.query.value_or(""))) {
                for (const auto &parameter: query->parameters) {
                    std::cerr << "Query parameter: '" << parameter.name << "', '" << parameter.value << "'" << std::endl;
                }
            } else {
                std::cerr << "Failed to parse query!" << std::endl;
            }
        } else {
            std::cerr << "Failed to parse url!" << std::endl;
        }

        if (auto resp = httplib::prepare_response(req)) {
            m_response = std::move(*resp);
        } else {
            reply(
                httplib::http_response_builder_t()
                .content_length(0)
                .connection_close()
                .build(httplib::response_status_from_error(resp.error()))
            );
            return;
        }

        if (auto reader = httplib::make_body_reader(req, m_bufstream)) {
            m_body_reader = std::move(*reader);
        } else {
            reply(
                m_response
                .content_length(0)
                .connection_close()
                .build(httplib::response_status_from_error(reader.error()))
            );
            return;
        }

        m_body_reader.async_read_some(boost::asio::buffer(m_read_buffer),
            [self = shared_from_this()](auto ec, auto transferred) {
                self->handle_body_read(ec, transferred);
            }
        );
    }

    void handle_body_read(boost::system::error_code ec, size_t transferred) {
        std::cerr << "Received body: " << escape_string(m_read_buffer.data(), transferred) << std::endl;

        if (ec) {
            std::cerr << "Error while read body: " << ec.message() << std::endl;
            reply(m_response.build(httplib::STATUS_200_OK));
            return;
        }

        m_body_reader.async_read_some(boost::asio::buffer(m_read_buffer),
            [self = shared_from_this()](auto ec, auto transferred) {
                self->handle_body_read(ec, transferred);
            }
        );
    }

    void reply(httplib::http_response_t reply) {
        m_write_buffer = boost::lexical_cast<std::string>(reply);

        boost::asio::async_write(m_socket, boost::asio::buffer(m_write_buffer),
            [self = shared_from_this()](auto ec, auto tr) {
                self->handle_write(ec, tr);
            }
        );
    }

    void handle_write(boost::system::error_code ec, size_t) {
        if (ec) {
            std::cerr << "Failed to reply: " << ec.message() << std::endl;
        } else {
            std::cerr << "Sent a reply" << std::endl;
        }
    }

public:
    boost::asio::ip::tcp::socket m_socket;

private:
    boost::asio::streambuf m_buffer;

    using buffered_stream_type = httplib::buffered_read_stream<boost::asio::ip::tcp::socket &, boost::asio::streambuf &>;

    buffered_stream_type m_bufstream;

    httplib::http_response_builder_t m_response;

    httplib::body_reader<buffered_stream_type> m_body_reader;
    std::array<char, 1024> m_read_buffer;
    std::string m_write_buffer;
};


class server_t {
public:
    server_t() :
        m_acceptor(m_io_service)
    { }

    void run() {
        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::address::from_string("127.0.0.1"),
            12345
        );

        m_acceptor.open(endpoint.protocol());
        m_acceptor.bind(endpoint);
        m_acceptor.listen(1);

        start_accept();

        m_io_service.run();
    }

private:
    void start_accept() {
        m_accepting = std::make_shared<connection_handler_t>(m_io_service);

        m_acceptor.async_accept(m_accepting->m_socket, [this](auto ec) {
            this->handle_accept(ec);
        });
    }

    void handle_accept(boost::system::error_code ec) {
        if (ec) {
            std::cerr << "Failed to accept: " << ec.message() << std::endl;
        } else {
            m_accepting->start();
            m_accepting.reset();
        }

        start_accept();
    }

private:
    boost::asio::io_service m_io_service;
    boost::asio::ip::tcp::acceptor m_acceptor;

    std::shared_ptr<connection_handler_t> m_accepting;
};


int main() {
    std::cerr << httplib::build_query({{
        {"abc def", "12 3"},
        {"!@#$%^&*()-_~=", "тест"},
        {"key", "value"}
    }}) << std::endl;


    auto url = httplib::normalize_url(
        *httplib::parse_url("HtTps://loCalhOst:443/./../abc/de/../?gfe_%72d=cr&q=%d1%82%d0%b5%D1%81%D1%82&abc&=def&asdf=#wwww")
    );

    std::cerr << "Schema: " << url.schema.value_or("none") << std::endl;
    std::cerr << "Host: " << url.host.value_or("none") << std::endl;

    if (url.port) {
        std::cerr << "Port: " << *url.port << std::endl;
    } else {
        std::cerr << "Port: none" << std::endl;
    }

    std::cerr << "Path: " << url.path << std::endl;
    std::cerr << "Query: " << url.query.value_or("none") << std::endl;
    std::cerr << "Fragment: " << url.fragment.value_or("none") << std::endl;


    if (auto query = httplib::parse_query(url.query.value_or(""))) {
        for (const auto &parameter: query->parameters) {
            std::cerr << "Query parameter: '" << parameter.name << "', '" << parameter.value << "'" << std::endl;
        }
    } else {
        std::cerr << "Failed to parse query!" << std::endl;
    }


    std::cerr << "Built up url: " << url << std::endl;


    return 0;


    server_t server;
    server.run();
    return 0;
}
