#pragma once

#include <httplib/detail/common.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>


HTTPLIB_OPEN_NAMESPACE


namespace detail {

    struct ilexicographical_less_t {
        struct is_transparent;

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

private:
    using container_t = std::map<header_name_t, header_values_t, detail::ilexicographical_less_t>;

public:
    using iterator = container_t::iterator;
    using const_iterator = container_t::const_iterator;
    using reverse_iterator = container_t::reverse_iterator;
    using const_reverse_iterator = container_t::const_reverse_iterator;

public:
    http_headers_t() :
        m_size(0)
    { }

    template<class It>
    http_headers_t(It begin, It end) :
        http_headers_t()
    {
        for (; begin != end; ++begin) {
            add_header_values(begin->first, begin->second);
        }
    }

    http_headers_t(std::initializer_list<container_t::value_type> list) :
        http_headers_t(list.begin(), list.end())
    { }

    http_headers_t(const http_headers_t &) = default;
    http_headers_t(http_headers_t &&) = default;

    http_headers_t &operator=(const http_headers_t &) = default;
    http_headers_t &operator=(http_headers_t &&) = default;

    bool empty() const {
        return size() == 0;
    }

    size_t size() const {
        return m_size;
    }

    iterator begin() {
        return m_headers.begin();
    }

    iterator end() {
        return m_headers.end();
    }

    const_iterator begin() const {
        return m_headers.begin();
    }

    const_iterator end() const {
        return m_headers.end();
    }

    const_iterator cbegin() const {
        return m_headers.cbegin();
    }

    const_iterator cend() const {
        return m_headers.cend();
    }

    reverse_iterator rbegin() {
        return m_headers.rbegin();
    }

    reverse_iterator rend() {
        return m_headers.rend();
    }

    const_reverse_iterator rbegin() const {
        return m_headers.rbegin();
    }

    const_reverse_iterator rend() const {
        return m_headers.rend();
    }

    const_reverse_iterator crbegin() const {
        return m_headers.crbegin();
    }

    const_reverse_iterator crend() const {
        return m_headers.crend();
    }

    iterator find(boost::string_view name) {
        return m_headers.find(name);
    }

    const_iterator find(boost::string_view name) const {
        return m_headers.find(name);
    }

    bool has(boost::string_view name) const {
        return static_cast<bool>(get_header_values(name));
    }

    boost::optional<const header_value_t &> get_header(boost::string_view name) const;
    boost::optional<const header_values_t &> get_header_values(boost::string_view name) const;

    void set_header(boost::string_view name, const header_values_t &values);
    void add_header_values(boost::string_view name, const header_values_t &values);
    void remove_header(boost::string_view name);

private:
    size_t m_size;
    container_t m_headers;
};


std::ostream &operator<<(std::ostream &stream, const http_headers_t &headers);


HTTPLIB_CLOSE_NAMESPACE
