#include <httplib/parser/request_parser.hpp>

#include <httplib/error.hpp>

#include <http_parser.h>


class httplib::http_request_parser_t::implementation_t {
public:
    http_parsing_options_t options;

    boost::system::error_code error;
    http_request_t request;

    http_headers_t::header_name_t current_header_name;
    http_headers_t::header_value_t current_header_value;

    enum class state_t {
        start,
        parsing_header_name,
        parsing_header_value,
        waiting_last_lf,
        done
    };

    state_t state;

    joyent::http_parser parser;
    joyent::http_parser_settings settings;

    implementation_t() :
        state(state_t::start)
    {
        joyent::http_parser_init(&parser, joyent::HTTP_REQUEST);
        joyent::http_parser_settings_init(&settings);

        settings.on_method = [](joyent::http_parser *parser, const char *data, size_t size) {
            return static_cast<implementation_t *>(parser->data)->handle_method(data, size);
        };

        settings.on_url = [](joyent::http_parser *parser, const char *data, size_t size) {
            return static_cast<implementation_t *>(parser->data)->handle_url(data, size);
        };

        settings.on_header_field = [](joyent::http_parser *parser, const char *data, size_t size) {
            return static_cast<implementation_t *>(parser->data)->handle_header_field(data, size);
        };

        settings.on_header_value = [](joyent::http_parser *parser, const char *data, size_t size) {
            return static_cast<implementation_t *>(parser->data)->handle_header_value(data, size);
        };

        settings.on_headers_complete = [](joyent::http_parser *parser) {
            return static_cast<implementation_t *>(parser->data)->handle_headers_complete();
        };

        settings.on_body = [](joyent::http_parser *parser, const char *data, size_t size) {
            return static_cast<implementation_t *>(parser->data)->handle_body(data, size);
        };

        settings.on_message_complete = [](joyent::http_parser *parser) {
            return static_cast<implementation_t *>(parser->data)->handle_message_complete();
        };
    }

    implementation_t(const implementation_t &other) = default;

    size_t parse(const char *data, size_t size) {
        if (state == state_t::done) {
            error = make_error_code(parser_errc_t::invalid_parser);
            return 0;
        }

        if (size == 0) {
            return 0;
        }

        std::size_t parsed = 0;

        if (state != state_t::waiting_last_lf) {
            parser.data = this;

            parsed = joyent::http_parser_execute(&parser, &settings, data, size);

            if (!error &&
                parser.http_errno != joyent::HPE_OK &&
                parser.http_errno != joyent::HPE_PAUSED)
            {
                error = boost::system::error_code(parser.http_errno, underlying_parser_category());
            }
        }

        if (!error && parsed < size && state == state_t::waiting_last_lf) {
            if (data[parsed] != '\n') {
                error = boost::system::error_code(joyent::HPE_LF_EXPECTED, underlying_parser_category());
            } else {
                ++parsed;
                state = state_t::done;
            }
        }

        if (error) {
            state = state_t::done;
        }

        return parsed;
    }

private:
    int handle_method(const char *data, size_t size) {
        request.method.append(data, size);
        return 0;
    }

    int handle_url(const char *data, size_t size) {
        request.url.append(data, size);

        if (request.url.size() > options.max_url_size) {
            error = make_error_code(parser_errc_t::too_long_url);
            return -1;
        }

        return 0;
    }

    int handle_header_field(const char *data, size_t size) {
        if (state == state_t::parsing_header_value) {
            request.headers.add_header_values(current_header_name, {std::move(current_header_value)});
            current_header_name.clear();
            current_header_value.clear();
        }

        state = state_t::parsing_header_name;

        current_header_name.append(data, size);

        if (request.headers.size() >= options.max_headers_number) {
            error = make_error_code(parser_errc_t::too_many_headers);
            return -1;
        }

        if (current_header_name.size() > options.max_header_size) {
            error = make_error_code(parser_errc_t::too_long_header);
            return -1;
        }

        return 0;
    }

    int handle_header_value(const char *data, size_t size) {
        state = state_t::parsing_header_value;

        current_header_value.append(data, size);

        if (current_header_name.size() + current_header_value.size() > options.max_header_size) {
            error = make_error_code(parser_errc_t::too_long_header);
            return -1;
        }
        return 0;
    }

    int handle_headers_complete() {
        if (state == state_t::parsing_header_value) {
            request.headers.add_header_values(current_header_name, {std::move(current_header_value)});
            current_header_name.clear();
            current_header_value.clear();
        }

        request.version.major = parser.http_major;
        request.version.minor = parser.http_minor;

        state = state_t::waiting_last_lf;

        joyent::http_parser_pause(&parser, 1);
        return 0;
    }

    int handle_body(const char *, size_t) {
        // We should not be here.
        // It seems handle_headers_complete() has not been called for some reason.
        assert(false);
        return 0;
    }

    int handle_message_complete() {
        // We should not be here.
        // It seems handle_headers_complete() has not been called for some reason.
        assert(false);
        return 0;
    }
};


httplib::http_request_parser_t::http_request_parser_t() :
    m_implementation(std::make_unique<implementation_t>())
{ }

httplib::http_request_parser_t::http_request_parser_t(const http_request_parser_t &other) :
    m_implementation(std::make_unique<implementation_t>(*other.m_implementation))
{ }

httplib::http_request_parser_t::http_request_parser_t(http_request_parser_t &&other) :
    m_implementation(std::move(other.m_implementation))
{ }

httplib::http_request_parser_t::~http_request_parser_t() { }

httplib::http_request_parser_t &httplib::http_request_parser_t::operator=(const http_request_parser_t &other) {
    m_implementation = std::make_unique<implementation_t>(*other.m_implementation);
    return *this;
}

httplib::http_request_parser_t &httplib::http_request_parser_t::operator=(http_request_parser_t &&other) {
    m_implementation = std::move(other.m_implementation);
    return *this;
}

void httplib::http_request_parser_t::set_options(http_parsing_options_t options) {
    m_implementation->options = options;
}

std::size_t httplib::http_request_parser_t::parse(const char *data, std::size_t size) {
    return m_implementation->parse(data, size);
}

bool httplib::http_request_parser_t::done() const {
    return m_implementation->state == implementation_t::state_t::done;
}

boost::system::error_code httplib::http_request_parser_t::error() const {
    return m_implementation->error;
}

httplib::http_request_t &httplib::http_request_parser_t::request() {
    return m_implementation->request;
}

const httplib::http_request_t &httplib::http_request_parser_t::request() const {
    return m_implementation->request;
}
