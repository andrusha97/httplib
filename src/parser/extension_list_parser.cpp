#include <httplib/parser/extension_list_parser.hpp>
#include <httplib/parser/detail/utility.hpp>

#include <boost/algorithm/string/predicate.hpp>

#include <algorithm>


HTTPLIB_OPEN_NAMESPACE


boost::optional<const extension_parameter_t &> extension_t::parameter(boost::string_view name) const {
    auto it = std::find_if(parameters.begin(), parameters.end(),
        [&name](const auto &v) {
            return v.name == name;
        }
    );

    if (it != parameters.end()) {
        return *it;
    } else {
        return boost::none;
    }
}


bool extension_t::equals(boost::string_view other) const {
 return boost::algorithm::iequals(name, other);
}


boost::optional<const extension_t &> extension_list_t::get(boost::string_view name) const {
    auto it = std::find_if(extensions.begin(), extensions.end(),
        [&name](const auto &v) {
            return v.equals(name);
        }
    );

    if (it != extensions.end()) {
        return *it;
    } else {
        return boost::none;
    }
}


namespace {

boost::optional<extension_parameter_t> parse_extension_parameter(boost::string_view &data) {
    boost::string_view leftover_data = data;

    extension_parameter_t result;

    if (auto name = detail::parse_token(leftover_data)) {
        result.name = std::move(*name);
    } else {
        return boost::none;
    }

    detail::skip_optional_whitespaces(leftover_data);

    if (leftover_data.empty() || leftover_data[0] != '=') {
        return boost::none;
    }

    leftover_data = leftover_data.substr(1);
    detail::skip_optional_whitespaces(leftover_data);

    if (leftover_data.empty()) {
        return boost::none;
    }

    boost::optional<std::string> value;

    if (leftover_data[0] == '"') {
        value = detail::parse_quoted_string(leftover_data);
    } else {
        value = detail::parse_token(leftover_data);
    }

    if (value) {
        data = leftover_data;
        result.value = std::move(*value);
        return result;
    } else {
        return boost::none;
    }
}

boost::optional<extension_t> parse_extension(boost::string_view &data) {
    boost::string_view leftover_data = data;

    extension_t result;

    if (auto token = detail::parse_token(leftover_data)) {
        result.name = std::move(*token);
    } else {
        return boost::none;
    }

    while (true) {
        detail::skip_optional_whitespaces(leftover_data);

        if (leftover_data.empty() || leftover_data[0] != ';') {
            data = leftover_data;
            return result;
        }

        leftover_data = leftover_data.substr(1);
        detail::skip_optional_whitespaces(leftover_data);

        if (auto parameter = parse_extension_parameter(leftover_data)) {
            result.parameters.emplace_back(std::move(*parameter));
        } else {
            return boost::none;
        }
    }
}

} // namespace


bool detail::parse_extension_list(boost::string_view data, extension_list_t &result) {
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

    if (auto extension = parse_extension(data)) {
        result.extensions.emplace_back(std::move(*extension));
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

        if (auto extension = parse_extension(data)) {
            result.extensions.emplace_back(std::move(*extension));
        } else {
            return false;
        }
    }
}

boost::optional<extension_list_t> parse_extension_list(boost::string_view data) {
    extension_list_t result;

    if (detail::parse_extension_list(data, result)) {
        return result;
    } else {
        return boost::none;
    }
}


HTTPLIB_CLOSE_NAMESPACE
