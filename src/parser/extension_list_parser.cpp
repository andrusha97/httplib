#include <httplib/parser/extension_list_parser.hpp>
#include <httplib/parser/detail/utility.hpp>

#include <boost/algorithm/string/predicate.hpp>

#include <algorithm>


boost::string_view httplib::extension_parameter_t::name() const {
    return m_name;
}

boost::string_view httplib::extension_parameter_t::value() const {
    return m_value;
}


boost::string_view httplib::extension_t::name() const {
    return m_name;
}

std::size_t httplib::extension_t::parameters_number() const {
    return m_parameters.size();
}

httplib::extension_t::const_iterator httplib::extension_t::parameters_begin() const {
    return m_parameters.begin();
}

httplib::extension_t::const_iterator httplib::extension_t::parameters_end() const {
    return m_parameters.end();
}

bool httplib::extension_t::has_parameter(boost::string_view name) const {
    auto it = std::find_if(m_parameters.begin(), m_parameters.end(),
        [&name](const auto &v) {
            return v.name() == name;
        }
    );

    return it != m_parameters.end();
}

boost::optional<const httplib::extension_parameter_t &> httplib::extension_t::parameter(boost::string_view name) const {
    auto it = std::find_if(m_parameters.begin(), m_parameters.end(),
        [&name](const auto &v) {
            return v.name() == name;
        }
    );

    if (it != m_parameters.end()) {
        return *it;
    } else {
        return boost::none;
    }
}

bool httplib::extension_t::equals(boost::string_view name) const {
 return boost::algorithm::iequals(m_name, name);
}

bool httplib::extension_t::operator==(boost::string_view name) const {
    return boost::algorithm::iequals(m_name, name);
}

bool httplib::extension_t::operator!=(boost::string_view name) const {
    return !boost::algorithm::iequals(m_name, name);
}


std::size_t httplib::extension_list_t::size() const {
    return m_extensions.size();
}

httplib::extension_list_t::const_iterator httplib::extension_list_t::begin() const {
    return m_extensions.begin();
}

httplib::extension_list_t::const_iterator httplib::extension_list_t::end() const {
    return m_extensions.end();
}

bool httplib::extension_list_t::has(boost::string_view name) const {
    auto it = std::find_if(m_extensions.begin(), m_extensions.end(),
        [&name](const auto &v) {
            return v.equals(name);
        }
    );

    return it != m_extensions.end();
}

boost::optional<const httplib::extension_t &> httplib::extension_list_t::get(boost::string_view name) const {
    auto it = std::find_if(m_extensions.begin(), m_extensions.end(),
        [&name](const auto &v) {
            return v.equals(name);
        }
    );

    if (it != m_extensions.end()) {
        return *it;
    } else {
        return boost::none;
    }
}


class httplib::detail::extension_parameter_access_t {
public:
    static std::string &name(extension_parameter_t &v) {
        return v.m_name;
    }

    static std::string &value(extension_parameter_t &v) {
        return v.m_value;
    }
};


class httplib::detail::extension_access_t {
public:
    static std::string &name(extension_t &v) {
        return v.m_name;
    }

    static extension_t::parameters_container_type &parameters(extension_t &v) {
        return v.m_parameters;
    }
};


class httplib::detail::extension_list_access_t {
public:
    static extension_list_t::extensions_container_type &extensions(extension_list_t &v) {
        return v.m_extensions;
    }
};


namespace {

boost::optional<httplib::extension_parameter_t> parse_extension_parameter(boost::string_view &data) {
    boost::string_view leftover_data = data;

    httplib::extension_parameter_t result;

    if (auto name = httplib::detail::parse_token(leftover_data)) {
        httplib::detail::extension_parameter_access_t::name(result) = std::move(*name);
    } else {
        return boost::none;
    }

    httplib::detail::skip_optional_whitespaces(leftover_data);

    if (leftover_data.empty() || leftover_data[0] != '=') {
        return boost::none;
    }

    leftover_data = leftover_data.substr(1);
    httplib::detail::skip_optional_whitespaces(leftover_data);

    if (leftover_data.empty()) {
        return boost::none;
    }

    boost::optional<std::string> value;

    if (leftover_data[0] == '"') {
        value = httplib::detail::parse_quoted_string(leftover_data);
    } else {
        value = httplib::detail::parse_token(leftover_data);
    }

    if (value) {
        data = leftover_data;
        httplib::detail::extension_parameter_access_t::value(result) = std::move(*value);
        return result;
    } else {
        return boost::none;
    }
}

boost::optional<httplib::extension_t> parse_extension(boost::string_view &data) {
    boost::string_view leftover_data = data;

    httplib::extension_t result;

    if (auto token = httplib::detail::parse_token(leftover_data)) {
        httplib::detail::extension_access_t::name(result) = std::move(*token);
    } else {
        return boost::none;
    }

    while (true) {
        httplib::detail::skip_optional_whitespaces(leftover_data);

        if (leftover_data.empty() || leftover_data[0] != ';') {
            data = leftover_data;
            return result;
        }

        leftover_data = leftover_data.substr(1);
        httplib::detail::skip_optional_whitespaces(leftover_data);

        if (auto parameter = parse_extension_parameter(leftover_data)) {
            httplib::detail::extension_access_t::parameters(result).emplace_back(std::move(*parameter));
        } else {
            return boost::none;
        }
    }
}

} // namespace


bool HTTPLIB_NAMESPACE::detail::parse_extension_list(boost::string_view data, extension_list_t &result) {
    while (true) {
        if (data.empty()) {
            return false;
        }

        if (data[0] == ',') {
            data = data.substr(1);
        } else {
            break;
        }

        httplib::detail::skip_optional_whitespaces(data);
    }

    if (auto extension = parse_extension(data)) {
        httplib::detail::extension_list_access_t::extensions(result).emplace_back(std::move(*extension));
    } else {
        return false;
    }

    while (true) {
        httplib::detail::skip_optional_whitespaces(data);

        if (data.empty()) {
            return true;
        }

        if (data[0] != ',') {
            return false;
        }

        data = data.substr(1);
        httplib::detail::skip_optional_whitespaces(data);

        if (data.empty() || data[0] == ',') {
            continue;
        }

        if (auto extension = parse_extension(data)) {
            httplib::detail::extension_list_access_t::extensions(result).emplace_back(std::move(*extension));
        } else {
            return false;
        }
    }
}

boost::optional<httplib::extension_list_t> HTTPLIB_NAMESPACE::parse_extension_list(boost::string_view data) {
    httplib::extension_list_t result;

    if (detail::parse_extension_list(data, result)) {
        return result;
    } else {
        return boost::none;
    }
}
