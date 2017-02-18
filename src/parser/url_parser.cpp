#include <httplib/parser/url_parser.hpp>

#include <http_parser.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>


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
        if (boost::algorithm::iequals(result.schema, "http") ||
            boost::algorithm::iequals(result.schema, "https"))
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


boost::string_view HTTPLIB_NAMESPACE::query_parameter_t::name() const {
    return m_name;
}


boost::string_view HTTPLIB_NAMESPACE::query_parameter_t::value() const {
    return m_value;
}


std::size_t HTTPLIB_NAMESPACE::query_t::size() const {
    return m_parameters.size();
}

HTTPLIB_NAMESPACE::query_t::const_iterator HTTPLIB_NAMESPACE::query_t::begin() const {
    return m_parameters.begin();
}

HTTPLIB_NAMESPACE::query_t::const_iterator HTTPLIB_NAMESPACE::query_t::end() const {
    return m_parameters.end();
}

bool HTTPLIB_NAMESPACE::query_t::has(boost::string_view name) const {
    return static_cast<bool>(get(name));
}

boost::optional<const HTTPLIB_NAMESPACE::query_parameter_t &> HTTPLIB_NAMESPACE::query_t::get(boost::string_view name) const {
    auto it = std::find_if(m_parameters.begin(), m_parameters.end(),
        [&name](const auto &parameter) {
            return parameter.name() == name;
        }
    );

    if (it != m_parameters.end()) {
        return *it;
    } else {
        return boost::none;
    }
}


namespace {

bool hex_digit_to_number(char digit, int &result) {
    if (digit >= '0' && digit <= '9') {
        result = digit - '0';
        return true;
    } else if (digit >= 'a' && digit <= 'f') {
        result = digit - 'a' + 10;
        return true;
    } else if (digit >= 'A' && digit <= 'F') {
        result = digit - 'A' + 10;
        return true;
    } else {
        return false;
    }
}

} // namespace


boost::optional<std::string> HTTPLIB_NAMESPACE::unescape_plus(boost::string_view data) {
    std::string result;

    for (auto it = data.begin(); it < data.end(); ++it) {
        if (*it == '+') {
            result.push_back(' ');
        } else if (*it == '%') {
            ++it;

            if (it == data.end()) {
                return boost::none;
            }

            int high_digit = 0;

            if (!hex_digit_to_number(*it, high_digit)) {
                return boost::none;
            }

            ++it;

            if (it == data.end()) {
                return boost::none;
            }

            int low_digit = 0;

            if (!hex_digit_to_number(*it, low_digit)) {
                return boost::none;
            }

            result.push_back(high_digit * 16 + low_digit);
        } else {
            result.push_back(*it);
        }
    }

    return result;
}


boost::optional<HTTPLIB_NAMESPACE::query_t> HTTPLIB_NAMESPACE::parse_query(boost::string_view data) {
    query_t result;

    size_t parameter_begin = 0;

    while (parameter_begin < data.size()) {
        size_t parameter_end = parameter_begin;

        while (parameter_end < data.size() && data[parameter_end] != '&') {
            ++parameter_end;
        }

        size_t delimiter = parameter_begin;

        while (delimiter < parameter_end && data[delimiter] != '=') {
            ++delimiter;
        }

        query_parameter_t parameter;

        if (auto name = unescape_plus(data.substr(parameter_begin, delimiter - parameter_begin))) {
            parameter.m_name = std::move(*name);
        } else {
            return boost::none;
        }

        if (delimiter + 1 < parameter_end) {
            if (auto value = unescape_plus(data.substr(delimiter + 1, parameter_end - delimiter - 1))) {
                parameter.m_value = std::move(*value);
            } else {
                return boost::none;
            }
        }

        result.m_parameters.push_back(std::move(parameter));

        parameter_begin = parameter_end + 1;
    }

    return result;
}
