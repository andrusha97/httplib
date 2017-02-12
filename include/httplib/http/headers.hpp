#pragma once

#include <httplib/detail/common.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/optional.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>


HTTPLIB_OPEN_NAMESPACE

namespace detail {

    struct ilexicographical_less_t {
        template<class One, class Another>
        bool operator()(const One &one, const Another &another) const {
            return boost::algorithm::ilexicographical_compare(one, another);
        }
    };

} // namespace detail


class http_headers_t {
public:
    using header_name_t = std::string;
    using header_value_t = std::string;
    using header_values_t = boost::container::small_vector<header_value_t, 3>;
    using container_t = std::map<header_name_t, header_values_t, detail::ilexicographical_less_t>;

public:
    http_headers_t();
    http_headers_t(container_t data);

    const container_t &data() const;
    std::size_t size() const;

    boost::optional<const header_value_t &> get_header(const header_name_t &name) const;
    boost::optional<const header_values_t &> get_header_values(const header_name_t &name) const;

    void set_header(const header_name_t &name, const header_values_t &values);
    void add_header_values(const header_name_t &name, const header_values_t &values);
    void remove_header(const header_name_t &name);

private:
    size_t m_size;
    container_t m_headers;
};


std::ostream &operator<<(std::ostream &stream, const http_headers_t &headers);

HTTPLIB_CLOSE_NAMESPACE
