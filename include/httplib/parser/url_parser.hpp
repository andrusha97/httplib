#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/http/url.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>


HTTPLIB_OPEN_NAMESPACE


boost::optional<url_t> parse_url(boost::string_view data);


class query_parameter_t;
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

    using container_type = boost::container::small_vector<query_parameter_t, 2>;

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


// Unescape percent-encoded string. If decode_plus is true, '+' is replaced with ' '.
boost::optional<std::string> unescape_plus(boost::string_view data);


// https://www.w3.org/TR/html5/forms.html#url-encoded-form-data
boost::optional<query_t> parse_query(boost::string_view data);


HTTPLIB_CLOSE_NAMESPACE
