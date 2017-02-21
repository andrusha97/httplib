#include <httplib/error.hpp>

#include <http_parser.h>


HTTPLIB_OPEN_NAMESPACE


namespace {

class parser_error_category_t : public boost::system::error_category {
public:
    const char *name() const noexcept override {
        return "http_parser";
    }

    std::string message(int code) const override {
        switch (code) {
            case static_cast<int>(parser_errc_t::too_long_header):
                return "Too long header";
            case static_cast<int>(parser_errc_t::too_many_headers):
                return "Too many headers";
            case static_cast<int>(parser_errc_t::too_long_url):
                return "Too long url";
            case static_cast<int>(parser_errc_t::too_long_reason):
                return "Too long reason";
            case static_cast<int>(parser_errc_t::invalid_parser):
                return "Invalid parser";
            default:
                return "HTTP parser error";
        }
    }

    bool equivalent(const boost::system::error_code &ec, int condition) const noexcept override {
        if (ec.category() == *this && ec.value() == condition) {
            return true;
        }

        return false;
    }
};

} // namespace

boost::system::error_code make_error_code(parser_errc_t e) noexcept {
    return boost::system::error_code(static_cast<int>(e), parser_category());
}

boost::system::error_condition make_error_condition(parser_errc_t e) noexcept {
    return boost::system::error_condition(static_cast<int>(e), parser_category());
}

const boost::system::error_category &parser_category() noexcept {
    static parser_error_category_t category;

    return category;
}


namespace {

class underlying_parser_error_category_t : public boost::system::error_category {
public:
    const char *name() const noexcept override {
        return "underlying_http_parser";
    }

    std::string message(int code) const override {
        if (code < 0 || code > joyent::HPE_UNKNOWN) {
            code = joyent::HPE_UNKNOWN;
        }

        return http_errno_description(static_cast<joyent::http_errno>(code));
    }
};

} // namespace


const boost::system::error_category &underlying_parser_category() noexcept {
    static underlying_parser_error_category_t category;

    return category;
}


namespace {

class reader_error_category_t : public boost::system::error_category {
public:
    const char *name() const noexcept override {
        return "reader";
    }

    std::string message(int code) const override {
        switch (code) {
            case static_cast<int>(reader_errc_t::eof):
                return "End of file";
            default:
                return "HTTP reader error";
        }
    }
};

} // namespace

boost::system::error_code make_error_code(reader_errc_t e) noexcept {
    return boost::system::error_code(static_cast<int>(e), reader_category());
}

boost::system::error_condition make_error_condition(reader_errc_t e) noexcept {
    return boost::system::error_condition(static_cast<int>(e), reader_category());
}

const boost::system::error_category &reader_category() noexcept {
    static reader_error_category_t category;

    return category;
}


HTTPLIB_CLOSE_NAMESPACE
