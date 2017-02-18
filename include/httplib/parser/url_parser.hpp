#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/http/url.hpp>

#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>

#include <vector>


HTTPLIB_OPEN_NAMESPACE


boost::optional<url_t> parse_url(boost::string_view data);


// https://tools.ietf.org/html/rfc3986#section-5.3
std::string build_url(const url_t &url);


// Unescape percent-encoded characters from the unreserved set,
// normalize all percent-encoded sequences to upper-case.
// https://tools.ietf.org/html/rfc3986#section-2.3
// https://tools.ietf.org/html/rfc3986#section-6
std::string normalize_percent_encoding(boost::string_view data);


// Remove segments '.' and '..'.
// https://tools.ietf.org/html/rfc3986#section-5.2.4
std::string normalize_path(boost::string_view path);


// Normalize percent-encoding of path and query, normalize dots in path, lower-case schema and host.
// If normalize_http is true, port 80 is removed for http scheme, and 443 for https.
// https://tools.ietf.org/html/rfc3986#section-6
url_t normalize_url(const url_t &url, bool normalize_http = true);


class query_t;

class query_parameter_t {
    friend boost::optional<query_t> parse_query(boost::string_view);

public:
    boost::string_view name() const;
    boost::string_view value() const;

private:
    std::string m_name;
    std::string m_value;
};


class query_t {
    friend boost::optional<query_t> parse_query(boost::string_view);

    using container_type = std::vector<query_parameter_t>;

public:
    using const_iterator = container_type::const_iterator;

public:
    std::size_t size() const;

    const_iterator begin() const;
    const_iterator end() const;

    bool has(boost::string_view name) const;
    boost::optional<const query_parameter_t &> get(boost::string_view name) const;

private:
    container_type m_parameters;
};


// Unescape percent-encoded string, '+' is replaced with ' '.
boost::optional<std::string> unescape(boost::string_view data);


// https://www.w3.org/TR/html5/forms.html#url-encoded-form-data
boost::optional<query_t> parse_query(boost::string_view data);


HTTPLIB_CLOSE_NAMESPACE
