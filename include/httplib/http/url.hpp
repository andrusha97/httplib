#pragma once

#include <httplib/detail/common.hpp>

#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>

#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>


HTTPLIB_OPEN_NAMESPACE


// http://localhost:123/a/b/c?arg1=val1&arg2=val2#frag
// http - schema
// localhost - host
// 123 - port
// /a/b/c - path
// arg1=val1&arg2=val2 - query
// frag - fragment
struct http_url_t {
    boost::optional<std::string> schema;
    boost::optional<std::string> host;
    boost::optional<uint16_t> port;
    std::string path;
    boost::optional<std::string> query;
    boost::optional<std::string> fragment;
};


struct query_parameter_t {
    std::string name;
    std::string value;
};


struct query_t {
    using container_type = std::vector<query_parameter_t>;

    container_type parameters;


    bool has(boost::string_view name) const {
        return static_cast<bool>(get(name));
    }

    boost::optional<const std::string &> get(boost::string_view name) const;
};


boost::optional<http_url_t> parse_url(boost::string_view data);


// https://tools.ietf.org/html/rfc3986#section-5.3
std::string build_url(const http_url_t &url);

std::ostream &operator<<(std::ostream &stream, const http_url_t &url);


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
http_url_t normalize_url(const http_url_t &url);


// Escape all characters except ones from the unreserved set.
std::string escape(boost::string_view data);


// Escape all characters except ones from the unreserved set. Spaces are replaced with '+'.
std::string escape_plus(boost::string_view data);


// Unescape percent-encoded string.
boost::optional<std::string> unescape(boost::string_view data);


// Unescape percent-encoded string, '+' is replaced with ' '.
boost::optional<std::string> unescape_plus(boost::string_view data);


// https://www.w3.org/TR/html5/forms.html#url-encoded-form-data
boost::optional<query_t> parse_query(boost::string_view data);


std::string build_query(const query_t &query);


HTTPLIB_CLOSE_NAMESPACE
