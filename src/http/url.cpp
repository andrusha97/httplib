#include <httplib/http/url.hpp>
#include <httplib/parser/detail/utility.hpp>

#include <http_parser.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

#include <cstring>
#include <vector>


boost::optional<const HTTPLIB_NAMESPACE::query_parameter_t &> HTTPLIB_NAMESPACE::query_t::get(boost::string_view name) const {
    auto it = std::find_if(parameters.begin(), parameters.end(),
        [&name](const auto &parameter) {
            return parameter.name == name;
        }
    );

    if (it != parameters.end()) {
        return *it;
    } else {
        return boost::none;
    }
}


boost::optional<httplib::url_t> HTTPLIB_NAMESPACE::parse_url(boost::string_view data) {
    joyent::http_parser_url parser;
    joyent::http_parser_url_init(&parser);

    if (joyent::http_parser_parse_url(data.data(), data.size(), 0, &parser) != 0) {
        return boost::none;
    }

    httplib::url_t result;

    if (parser.field_set & (1 << joyent::UF_SCHEMA)) {
        result.schema = data.substr(parser.field_data[joyent::UF_SCHEMA].off,
                                    parser.field_data[joyent::UF_SCHEMA].len).to_string();
    }

    if (parser.field_set & (1 << joyent::UF_USERINFO)) {
        // RFC 7230 forbids user info in http and https url's.
        if (result.schema &&
            (boost::algorithm::iequals(*result.schema, "http") ||
             boost::algorithm::iequals(*result.schema, "https")))
        {
            return boost::none;
        }

        result.user_info = data.substr(parser.field_data[joyent::UF_USERINFO].off,
                                       parser.field_data[joyent::UF_USERINFO].len).to_string();
    }

    if (parser.field_set & (1 << joyent::UF_HOST)) {
        result.host = data.substr(parser.field_data[joyent::UF_HOST].off,
                                  parser.field_data[joyent::UF_HOST].len).to_string();
    }

    if (parser.field_set & (1 << joyent::UF_PORT)) {
        if (parser.field_data[joyent::UF_PORT].len > 0) {
            uint16_t port = 0;

            bool port_converison_result = boost::conversion::try_lexical_convert(
                data.data() + parser.field_data[joyent::UF_PORT].off,
                parser.field_data[joyent::UF_PORT].len,
                port
            );

            if (!port_converison_result) {
                return boost::none;
            }

            result.port = port;
        }
    }

    if (parser.field_set & (1 << joyent::UF_PATH)) {
        result.path = data.substr(parser.field_data[joyent::UF_PATH].off,
                                  parser.field_data[joyent::UF_PATH].len).to_string();
    }

    if (parser.field_set & (1 << joyent::UF_QUERY)) {
        result.query = data.substr(parser.field_data[joyent::UF_QUERY].off,
                                   parser.field_data[joyent::UF_QUERY].len).to_string();
    }

    if (parser.field_set & (1 << joyent::UF_FRAGMENT)) {
        result.fragment = data.substr(parser.field_data[joyent::UF_FRAGMENT].off,
                                      parser.field_data[joyent::UF_FRAGMENT].len).to_string();
    }

    return result;
}


std::string HTTPLIB_NAMESPACE::build_url(const url_t &url) {
    // https://tools.ietf.org/html/rfc3986#section-5.3

    std::string result;

    if (url.schema) {
        result += *url.schema;
        result += ":";
    }

    if (url.user_info || url.host || url.port) {
        result += "//";

        if (url.user_info) {
            result += *url.user_info;
            result += "@";
        }

        if (url.host) {
            result += *url.host;
        }

        if (url.port) {
            result += ":";
            result += std::to_string(*url.port);
        }
    }

    result += url.path;

    if (url.query) {
        result += "?";
        result += *url.query;
    }

    if (url.fragment) {
        result += "#";
        result += *url.fragment;
    }

    return result;
}


std::ostream &HTTPLIB_NAMESPACE::operator<<(std::ostream &stream, const url_t &url) {
    // The same algorithm as build_url. https://tools.ietf.org/html/rfc3986#section-5.3
    if (url.schema) {
        stream << *url.schema << ":";
    }

    if (url.user_info || url.host || url.port) {
        stream << "//";

        if (url.user_info) {
            stream << *url.user_info << "@";
        }

        if (url.host) {
            stream << *url.host;
        }

        if (url.port) {
            stream << ":" << *url.port;
        }
    }

    stream << url.path;

    if (url.query) {
        stream << "?" << *url.query;
    }

    if (url.fragment) {
        stream << "#" << *url.fragment;
    }

    return stream;
}


namespace {

bool hex_digit_to_number(unsigned char digit, unsigned int &result) {
    if (digit >= static_cast<unsigned char>('0') && digit <= static_cast<unsigned char>('9')) {
        result = digit - static_cast<unsigned char>('0');
        return true;
    } else if (digit >= static_cast<unsigned char>('a') && digit <= static_cast<unsigned char>('f')) {
        result = digit - static_cast<unsigned char>('a') + 10;
        return true;
    } else if (digit >= static_cast<unsigned char>('A') && digit <= static_cast<unsigned char>('F')) {
        result = digit - static_cast<unsigned char>('A') + 10;
        return true;
    } else {
        return false;
    }
}

bool is_unreserved(unsigned char ch) {
    return HTTPLIB_NAMESPACE::detail::is_alpha(ch) ||
           HTTPLIB_NAMESPACE::detail::is_digit(ch) ||
           ch == '-' ||
           ch == '.' ||
           ch == '_' ||
           ch == '~';
}

unsigned char to_upper_case(unsigned char ch) {
    if (ch >= 0x61 && ch <= 0x7A) { // a-z
        return ch - 0x61 + 0x41; // to A-Z
    } else {
        return ch;
    }
}

unsigned char to_lower_case(unsigned char ch) {
    if (ch >= 0x41 && ch <= 0x5A) { // A-Z
        return ch - 0x41 + 0x61; // to a-z
    } else {
        return ch;
    }
}

std::string to_lower_case(boost::string_view str) {
    std::string result;
    result.reserve(str.size());

    for (auto ch: str) {
        result.push_back(to_lower_case(ch));
    }

    return result;
}

} // namespace


std::string HTTPLIB_NAMESPACE::normalize_percent_encoding(boost::string_view data) {
    std::string result;

    for (auto it = data.begin(); it < data.end(); ++it) {
        if (*it == '%') {
            auto escape_sequence_it = it;
            ++escape_sequence_it;

            if (escape_sequence_it == data.end()) {
                result.push_back(*it);
                continue;
            }

            char high_digit_char = *escape_sequence_it;
            unsigned int high_digit = 0;

            if (!hex_digit_to_number(high_digit_char, high_digit)) {
                result.push_back(*it);
                continue;
            }

            ++escape_sequence_it;

            if (escape_sequence_it == data.end()) {
                result.push_back(*it);
                continue;
            }

            char low_digit_char = *escape_sequence_it;
            unsigned int low_digit = 0;

            if (!hex_digit_to_number(low_digit_char, low_digit)) {
                result.push_back(*it);
                continue;
            }

            unsigned char unescaped = high_digit * 16 + low_digit;

            if (is_unreserved(unescaped)) {
                result.push_back(unescaped);
            } else {
                result.push_back('%');
                result.push_back(to_upper_case(high_digit_char));
                result.push_back(to_upper_case(low_digit_char));
            }

            it = escape_sequence_it;
        } else {
            result.push_back(*it);
        }
    }

    return result;
}


std::string HTTPLIB_NAMESPACE::normalize_path(boost::string_view path) {
    // Implementation of https://tools.ietf.org/html/rfc3986#section-5.2.4

    std::vector<boost::string_view> segments;

    while (!path.empty()) {
        if (path.starts_with("./")) {
            path = path.substr(std::strlen("./"));
        } else if (path.starts_with("../")) {
            path = path.substr(std::strlen("../"));
        } else if (path.starts_with("/./")) {
            path = path.substr(std::strlen("/."));
        } else if (path == "/.") {
            // Replace it with the last empty segment.
            segments.push_back(path.substr(0, std::strlen("/")));
            path = boost::string_view();
        } else if (path.starts_with("/../")) {
            if (!segments.empty()) {
                segments.pop_back();
            }

            path = path.substr(std::strlen("/.."));
        } else if (path == "/..") {
            if (!segments.empty()) {
                segments.pop_back();
            }

            // Replace it with the last empty segment.
            segments.push_back(path.substr(0, std::strlen("/")));
            path = boost::string_view();
        } else if (path == ".") {
            path = boost::string_view();
        } else if (path == "..") {
            path = boost::string_view();
        } else {
            auto segment_end = path.find('/', 1);
            segments.push_back(path.substr(0, segment_end));

            if (segment_end != boost::string_view::npos) {
                path = path.substr(segment_end);
            } else {
                path = boost::string_view();
            }
        }
    }

    std::string result;

    for (const auto &segment: segments) {
        result.append(segment.data(), segment.size());
    }

    return result;
}


HTTPLIB_NAMESPACE::url_t HTTPLIB_NAMESPACE::normalize_url(const url_t &url, bool normalize_http) {
    url_t result;

    if (url.schema) {
        result.schema = to_lower_case(*url.schema);
    }

    result.user_info = url.user_info;

    if (url.host) {
        result.host = to_lower_case(*url.host);
    }

    result.port = url.port;
    result.path = normalize_percent_encoding(normalize_path(url.path));

    if (url.query) {
        result.query = normalize_percent_encoding(*url.query);
    }

    result.fragment = url.fragment;

    if (normalize_http) {
        if (result.port && result.schema) {
            if (*result.port == 80 && *result.schema == "http") {
                result.port = boost::none;
            } else if (*result.port == 443 && *result.schema == "https") {
                result.port = boost::none;
            }
        }

        if (result.path.empty()) {
            result.path = "/";
        }
    }

    return result;
}


namespace {

boost::optional<std::string> unescape_impl(boost::string_view data, bool plus) {
    std::string result;

    for (auto it = data.begin(); it < data.end(); ++it) {
        if (plus && *it == '+') {
            result.push_back(' ');
        } else if (*it == '%') {
            ++it;

            if (it == data.end()) {
                return boost::none;
            }

            unsigned int high_digit = 0;

            if (!hex_digit_to_number(*it, high_digit)) {
                return boost::none;
            }

            ++it;

            if (it == data.end()) {
                return boost::none;
            }

            unsigned int low_digit = 0;

            if (!hex_digit_to_number(*it, low_digit)) {
                return boost::none;
            }

            result.push_back(static_cast<unsigned char>(high_digit * 16 + low_digit));
        } else {
            result.push_back(*it);
        }
    }

    return result;
}

} // namespace


boost::optional<std::string> HTTPLIB_NAMESPACE::unescape(boost::string_view data) {
    return unescape_impl(data, false);
}


boost::optional<std::string> HTTPLIB_NAMESPACE::unescape_plus(boost::string_view data) {
    return unescape_impl(data, true);
}


boost::optional<HTTPLIB_NAMESPACE::query_t> HTTPLIB_NAMESPACE::parse_query(boost::string_view query) {
    query_t result;

    while (!query.empty()) {
        // W3C recommends supporting both semicolon and ampersand as delimiters.
        // https://www.w3.org/TR/html401/appendix/notes.html#ampersands-in-uris
        // Here too: https://tools.ietf.org/html/rfc1866#section-8.2.1
        auto parameter_string = query.substr(0, query.find_first_of("&;"));
        query.remove_prefix(parameter_string.size() + 1);

        auto name_string = parameter_string.substr(0, parameter_string.find('='));

        query_parameter_t parameter;

        if (auto parsed_name = unescape_plus(name_string)) {
            parameter.name = std::move(*parsed_name);
        } else {
            return boost::none;
        }

        parameter_string.remove_prefix(name_string.size() + 1);

        if (auto parsed_value = unescape_plus(parameter_string)) {
            parameter.value = std::move(*parsed_value);
        } else {
            return boost::none;
        }

        result.parameters.push_back(std::move(parameter));
    }

    return result;
}


namespace {

std::string escape_impl(boost::string_view data, bool plus) {
    // FIXME:
    // This algorithm escapes all but unreserved characters according to RFC 3986.
    // But https://www.w3.org/TR/html5/forms.html#url-encoded-form-data defines slightly different
    // set of characters to leave unescaped.
    // What is the right way to implement this?

    std::string result;

    const char *hex = "0123456789ABCDEF";

    for (auto it = data.begin(); it < data.end(); ++it) {
        if (plus && *it == ' ') {
            result.push_back('+');
        } else if (is_unreserved(*it)) {
            result.push_back(*it);
        } else {
            unsigned char uch = *it;
            result.push_back('%');
            result.push_back(hex[uch / 16]);
            result.push_back(hex[uch % 16]);
        }
    }

    return result;
}

} // namespace


std::string HTTPLIB_NAMESPACE::escape(boost::string_view data) {
    return escape_impl(data, false);
}


std::string HTTPLIB_NAMESPACE::escape_plus(boost::string_view data) {
    return escape_impl(data, true);
}


std::string HTTPLIB_NAMESPACE::build_query(const query_t &query) {
    std::string result;
    bool first = true;

    for (const auto &parameter: query.parameters) {
        if (!first) {
            result.push_back('&');
        } else {
            first = false;
        }

        result += escape_plus(parameter.name);
        result.push_back('=');
        result += escape_plus(parameter.value);
    }

    return result;
}
