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
