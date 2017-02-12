#pragma once

#include <httplib/detail/common.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>

#include <cstdlib>
#include <string>


HTTPLIB_OPEN_NAMESPACE

class extension_list_t;


namespace detail {

class extension_parameter_access_t;
class extension_access_t;
class extension_list_access_t;

bool parse_extension_list(boost::string_view data, extension_list_t &result);

} // namespace detail


// token BWS "=" BWS ( token / quoted-string )
class extension_parameter_t {
    friend class detail::extension_parameter_access_t;

public:
    boost::string_view name() const;
    boost::string_view value() const;

private:
    std::string m_name;
    std::string m_value;
};


// token *( OWS ";" OWS extension-parameter )
class extension_t {
    friend class detail::extension_access_t;

    using parameters_container_type = boost::container::small_vector<extension_parameter_t, 1>;

public:
    using const_iterator = parameters_container_type::const_iterator;

public:
    boost::string_view name() const;

    std::size_t parameters_number() const;

    const_iterator parameters_begin() const;
    const_iterator parameters_end() const;

    bool has_parameter(boost::string_view name) const;
    boost::optional<const extension_parameter_t &> parameter(boost::string_view name) const;

    // Compare name case-insensitively.
    bool equals(boost::string_view name) const;
    bool operator==(boost::string_view name) const;
    bool operator!=(boost::string_view name) const;

private:
    std::string m_name;
    parameters_container_type m_parameters;
};


// *( "," OWS ) extension *( OWS "," [ OWS extension ] )
class extension_list_t {
    friend class detail::extension_list_access_t;

    using extensions_container_type = boost::container::small_vector<extension_t, 2>;

public:
    using const_iterator = extensions_container_type::const_iterator;

public:
    std::size_t size() const;

    const_iterator begin() const;
    const_iterator end() const;

    bool has(boost::string_view name) const;
    boost::optional<const extension_t &> get(boost::string_view name) const;

private:
    extensions_container_type m_extensions;
};


// Parse a list from a header value.
boost::optional<extension_list_t> parse_extension_list(boost::string_view data);


// Parse a list from a sequence of header values.
template<class It>
boost::optional<extension_list_t> parse_extension_list(It begin, It end) {
    if (begin == end) {
        return boost::none;
    }

    extension_list_t result;

    for (; begin != end; ++begin) {
        if (!detail::parse_extension_list(*begin, result)) {
            return boost::none;
        }
    }

    return result;
}


HTTPLIB_CLOSE_NAMESPACE
