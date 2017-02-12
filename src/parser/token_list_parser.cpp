#include <httplib/parser/token_list_parser.hpp>
#include <httplib/parser/detail/utility.hpp>

#include <boost/algorithm/string/predicate.hpp>


boost::string_view httplib::token_t::value() const {
    return m_value;
}

bool httplib::token_t::equals(boost::string_view value) const {
 return boost::algorithm::iequals(m_value, value);
}

bool httplib::token_t::operator==(boost::string_view value) const {
    return boost::algorithm::iequals(m_value, value);
}

bool httplib::token_t::operator!=(boost::string_view value) const {
    return !boost::algorithm::iequals(m_value, value);
}


std::size_t httplib::token_list_t::size() const {
    return m_tokens.size();
}

httplib::token_list_t::const_iterator httplib::token_list_t::begin() const {
    return m_tokens.begin();
}

httplib::token_list_t::const_iterator httplib::token_list_t::end() const {
    return m_tokens.end();
}

bool httplib::token_list_t::has(boost::string_view token) const {
    auto it = std::find_if(m_tokens.begin(), m_tokens.end(),
        [&token](const auto &v) {
            return v.equals(token);
        }
    );

    return it != m_tokens.end();
}


class httplib::detail::token_access_t {
public:
    static std::string &value(token_t &v) {
        return v.m_value;
    }
};


class httplib::detail::token_list_access_t {
public:
    static token_list_t::container_type &tokens(token_list_t &v) {
        return v.m_tokens;
    }
};


bool HTTPLIB_NAMESPACE::detail::parse_token_list(boost::string_view data, token_list_t &result) {
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
        token_t token;
        detail::token_access_t::value(token) = std::move(*token_value);
        detail::token_list_access_t::tokens(result).emplace_back(std::move(token));
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
            token_t token;
            detail::token_access_t::value(token) = std::move(*token_value);
            detail::token_list_access_t::tokens(result).emplace_back(std::move(token));
        } else {
            return false;
        }
    }
}


boost::optional<httplib::token_list_t> HTTPLIB_NAMESPACE::parse_token_list(boost::string_view data) {
    token_list_t result;

    if (detail::parse_token_list(data, result)) {
        return result;
    } else {
        return boost::none;
    }
}
