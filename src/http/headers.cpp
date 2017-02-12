#include <httplib/http/headers.hpp>


httplib::http_headers_t::http_headers_t() :
    m_size(0)
{ }

httplib::http_headers_t::http_headers_t(container_t data) :
    m_size(0),
    m_headers(std::move(data))
{
    for (const auto &header: m_headers) {
        m_size += header.second.size();
    }
}

const httplib::http_headers_t::container_t &httplib::http_headers_t::data() const {
    return m_headers;
}

std::size_t httplib::http_headers_t::size() const {
    return m_size;
}

boost::optional<const httplib::http_headers_t::header_value_t &>
httplib::http_headers_t::get_header(const header_name_t &name) const {
    auto header_it = m_headers.find(name);

    if (header_it != m_headers.end() && header_it->second.size() == 1) {
        return header_it->second.front();
    } else {
        return boost::none;
    }
}

boost::optional<const httplib::http_headers_t::header_values_t &>
httplib::http_headers_t::get_header_values(const header_name_t &name) const {
    auto header_it = m_headers.find(name);

    if (header_it != m_headers.end()) {
        return header_it->second;
    } else {
        return boost::none;
    }
}

void httplib::http_headers_t::set_header(const header_name_t &name, const header_values_t &values) {
    auto result = m_headers.emplace(name, header_values_t());
    size_t headers_to_replace = result.first->second.size();

    result.first->second = values;

    m_size -= headers_to_replace;
    m_size += result.first->second.size();
}

void httplib::http_headers_t::add_header_values(const header_name_t &name, const header_values_t &values) {
    auto result = m_headers.emplace(name, header_values_t());

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

void httplib::http_headers_t::remove_header(const header_name_t &name) {
    auto header_it = m_headers.find(name);

    if (header_it != m_headers.end()) {
        size_t headers_to_remove = header_it->second.size();

        m_headers.erase(header_it);
        m_size -= headers_to_remove;
    }
}


std::ostream &HTTPLIB_NAMESPACE::operator<<(std::ostream &stream, const http_headers_t &headers) {
    const auto &data = headers.data();
    auto home_it = data.find("home");

    // Print the Home header first, as it's recommended by rfc7230.
    if (home_it != data.end()) {
        for (const auto &value: home_it->second) {
            stream << home_it->first << ": " << value << "\r\n";
        }
    }

    for (auto header_it = data.begin(); header_it != data.end(); ++header_it) {
        if (header_it != home_it) {
            for (const auto &value: header_it->second) {
                stream << header_it->first << ": " << value << "\r\n";
            }
        }
    }

    return stream;
}
