#include <httplib/parser/token_list_parser.hpp>
#include <httplib/parser/detail/utility.hpp>

#include <boost/algorithm/string/predicate.hpp>


HTTPLIB_OPEN_NAMESPACE


bool token_t::equals(boost::string_view other) const {
 return boost::algorithm::iequals(value, other);
}


bool token_list_t::has(boost::string_view token) const {
    auto it = std::find_if(tokens.begin(), tokens.end(),
        [&token](const auto &v) {
            return v.equals(token);
        }
    );

    return it != tokens.end();
}


bool detail::parse_token_list(boost::string_view data, token_list_t &result) {
    while (true) {
        if (data.empty()) {
            return false;
        }

        if (data[0] == ',') {
            data = data.substr(1);
        } else {
            break;
        }

        detail::skip_optional_whitespaces(data);
    }

    if (auto token_value = detail::parse_token(data)) {
        result.tokens.emplace_back(token_t {std::move(*token_value)});
    } else {
        return false;
    }

    while (true) {
        detail::skip_optional_whitespaces(data);

        if (data.empty()) {
            return true;
        }

        if (data[0] != ',') {
            return false;
        }

        data = data.substr(1);
        detail::skip_optional_whitespaces(data);

        if (data.empty() || data[0] == ',') {
            continue;
        }

        if (auto token_value = detail::parse_token(data)) {
            result.tokens.emplace_back(token_t {std::move(*token_value)});
        } else {
            return false;
        }
    }
}


boost::optional<token_list_t> parse_token_list(boost::string_view data) {
    token_list_t result;

    if (detail::parse_token_list(data, result)) {
        return result;
    } else {
        return boost::none;
    }
}


HTTPLIB_CLOSE_NAMESPACE
