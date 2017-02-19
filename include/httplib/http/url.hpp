#pragma once

#include <httplib/detail/common.hpp>

#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>

#include <stdint.h>
#include <string>
#include <vector>


HTTPLIB_OPEN_NAMESPACE


// http://user:pass@localhost:123/a/b/c?arg1=val1&arg2=val2#frag
// http - schema
// user:pass - user_info
// localhost - host
// 123 - port
// /a/b/c - path
// arg1=val1&arg2=val2 - query
// frag - fragment
struct url_t {
    boost::optional<std::string> schema;
    boost::optional<std::string> user_info;
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

    boost::optional<const query_parameter_t &> get(boost::string_view name) const;
};


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


// Unescape percent-encoded string, '+' is replaced with ' '.
boost::optional<std::string> unescape(boost::string_view data);


// https://www.w3.org/TR/html5/forms.html#url-encoded-form-data
boost::optional<query_t> parse_query(boost::string_view data);


HTTPLIB_CLOSE_NAMESPACE
