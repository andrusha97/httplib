#include <httplib/parser/chunked_body_parser.hpp>

#include <httplib/error.hpp>

#include <http_parser.h>


HTTPLIB_OPEN_NAMESPACE


class chunked_body_parser_t::implementation_t {
public:
    http_parsing_options_t options;

    boost::system::error_code error;
    const char *body_part;
    std::size_t body_part_size;

    http_headers_t headers;

    http_headers_t::header_name_t current_header_name;
    http_headers_t::header_value_t current_header_value;

    enum class state_t {
        start,
        parsing_header_name,
        parsing_header_value,
        done
    };

    state_t state;

    joyent::http_parser parser;
    joyent::http_parser_settings settings;

    implementation_t() :
        body_part(nullptr),
        body_part_size(0),
        state(state_t::start)
    {
        joyent::http_parser_init(&parser, joyent::HTTP_CHUNKED_BODY);
        joyent::http_parser_settings_init(&settings);

        settings.on_header_field = [](joyent::http_parser *parser, const char *data, size_t size) {
            return static_cast<implementation_t *>(parser->data)->handle_header_field(data, size);
        };

        settings.on_header_value = [](joyent::http_parser *parser, const char *data, size_t size) {
            return static_cast<implementation_t *>(parser->data)->handle_header_value(data, size);
        };

        settings.on_body = [](joyent::http_parser *parser, const char *data, size_t size) {
            return static_cast<implementation_t *>(parser->data)->handle_body(data, size);
        };

        settings.on_message_complete = [](joyent::http_parser *parser) {
            return static_cast<implementation_t *>(parser->data)->handle_message_complete();
        };
    }

    implementation_t(const implementation_t &other) = default;

    chunked_body_parser_t::result_t parse(const char *data, size_t size) {
        if (state == state_t::done) {
            error = make_error_code(parser_errc_t::invalid_parser);
            return {0, chunked_body_parser_t::error_t{error}};
        }

        if (size == 0) {
            return {0, chunked_body_parser_t::none_t{}};
        }

        body_part = nullptr;
        body_part_size = 0;

        std::size_t parsed = 0;

        parser.data = this;

        if (parser.http_errno == joyent::HPE_PAUSED) {
            joyent::http_parser_pause(&parser, 0);
        }

        parsed = joyent::http_parser_execute(&parser, &settings, data, size);

        if (!error &&
            parser.http_errno != joyent::HPE_OK &&
            parser.http_errno != joyent::HPE_PAUSED)
        {
            error = boost::system::error_code(parser.http_errno, underlying_parser_category());
        }

        if (error) {
            state = state_t::done;
            return {parsed, chunked_body_parser_t::error_t{error}};
        }

        if (body_part) {
            return {parsed, chunked_body_parser_t::data_t{body_part, body_part_size}};
        }

        return {parsed, chunked_body_parser_t::none_t{}};
    }

private:
    int handle_header_field(const char *data, size_t size) {
        if (state == state_t::parsing_header_value) {
            headers.add_header_values(current_header_name, {std::move(current_header_value)});
            current_header_name.clear();
            current_header_value.clear();
        }

        state = state_t::parsing_header_name;

        current_header_name.append(data, size);

        if (headers.size() >= options.max_headers_number) {
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

    int handle_body(const char *data, std::size_t size) {
        body_part = data;
        body_part_size = size;

        joyent::http_parser_pause(&parser, 1);

        return 0;
    }

    int handle_message_complete() {
        if (state == state_t::parsing_header_value) {
            headers.add_header_values(current_header_name, {std::move(current_header_value)});
            current_header_name.clear();
            current_header_value.clear();
        }

        state = state_t::done;

        joyent::http_parser_pause(&parser, 1);

        return 0;
    }
};


chunked_body_parser_t::chunked_body_parser_t() :
    m_implementation(std::make_unique<implementation_t>())
{ }

chunked_body_parser_t::chunked_body_parser_t(const chunked_body_parser_t &other) :
    m_implementation(std::make_unique<implementation_t>(*other.m_implementation))
{ }

chunked_body_parser_t::chunked_body_parser_t(chunked_body_parser_t &&other) :
    m_implementation(std::move(other.m_implementation))
{ }

chunked_body_parser_t::~chunked_body_parser_t() { }

chunked_body_parser_t &chunked_body_parser_t::operator=(const chunked_body_parser_t &other) {
    m_implementation = std::make_unique<implementation_t>(*other.m_implementation);
    return *this;
}

chunked_body_parser_t &chunked_body_parser_t::operator=(chunked_body_parser_t &&other) {
    m_implementation = std::move(other.m_implementation);
    return *this;
}

void chunked_body_parser_t::set_options(http_parsing_options_t options) {
    m_implementation->options = options;
}

chunked_body_parser_t::result_t chunked_body_parser_t::parse(const char *data, std::size_t size) {
    return m_implementation->parse(data, size);
}

bool chunked_body_parser_t::done() const {
    return m_implementation->state == implementation_t::state_t::done;
}

http_headers_t &chunked_body_parser_t::headers() {
    return m_implementation->headers;
}

const http_headers_t &chunked_body_parser_t::headers() const {
    return m_implementation->headers;
}


HTTPLIB_CLOSE_NAMESPACE
