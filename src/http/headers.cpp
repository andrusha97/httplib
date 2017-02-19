#include <httplib/http/headers.hpp>


boost::optional<const httplib::http_headers_t::header_value_t &>
httplib::http_headers_t::get_header(boost::string_view name) const {
    auto header_it = m_headers.find(name);

    if (header_it != m_headers.end() && header_it->second.size() == 1) {
        return header_it->second.front();
    } else {
        return boost::none;
    }
}


boost::optional<const httplib::http_headers_t::header_values_t &>
httplib::http_headers_t::get_header_values(boost::string_view name) const {
    auto header_it = m_headers.find(name);

    if (header_it != m_headers.end()) {
        return header_it->second;
    } else {
        return boost::none;
    }
}


void httplib::http_headers_t::set_header(boost::string_view name, const header_values_t &values) {
    if (values.empty()) {
        remove_header(name);
    } else {
        auto result = m_headers.emplace(name.to_string(), header_values_t());
        size_t headers_to_replace = result.first->second.size();

        result.first->second = values;

        m_size -= headers_to_replace;
        m_size += result.first->second.size();
    }
}


void httplib::http_headers_t::add_header_values(boost::string_view name, const header_values_t &values) {
    if (values.empty()) {
        return;
    }

    auto result = m_headers.emplace(name.to_string(), header_values_t());

    if (result.first->second.empty()) {
        result.first->second = values;
        m_size += result.first->second.size();
    } else {
        result.first->second.reserve(result.first->second.size() + values.size());

        for (auto &value: values) {
            result.first->second.emplace_back(value);
            ++m_size;
        }
    }
}


void httplib::http_headers_t::remove_header(boost::string_view name) {
    auto header_it = m_headers.find(name);

    if (header_it != m_headers.end()) {
        size_t headers_to_remove = header_it->second.size();

        m_headers.erase(header_it);
        m_size -= headers_to_remove;
    }
}


std::ostream &HTTPLIB_NAMESPACE::operator<<(std::ostream &stream, const http_headers_t &headers) {
    auto home_it = headers.find("home");

    // Print the Home header first, as it's recommended by rfc7230.
    if (home_it != headers.end()) {
        for (const auto &value: home_it->second) {
            stream << home_it->first << ": " << value << "\r\n";
        }
    }

    for (auto header_it = headers.begin(); header_it != headers.end(); ++header_it) {
        if (header_it != home_it) {
            for (const auto &value: header_it->second) {
                stream << header_it->first << ": " << value << "\r\n";
            }
        }
    }

    return stream;
}
