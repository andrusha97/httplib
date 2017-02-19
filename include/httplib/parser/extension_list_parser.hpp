#pragma once

#include <httplib/detail/common.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>

#include <cstdlib>
#include <string>


HTTPLIB_OPEN_NAMESPACE


// token BWS "=" BWS ( token / quoted-string )
struct extension_parameter_t {
    std::string name;
    std::string value;
};


// token *( OWS ";" OWS extension-parameter )
struct extension_t {
    using parameters_container_type = boost::container::small_vector<extension_parameter_t, 1>;

    std::string name;
    parameters_container_type parameters;


    bool has_parameter(boost::string_view name) const {
        return static_cast<bool>(parameter(name));
    }

    boost::optional<const extension_parameter_t &> parameter(boost::string_view name) const;

    // Compare name case-insensitively.
    bool equals(boost::string_view name) const;
};


// *( "," OWS ) extension *( OWS "," [ OWS extension ] )
struct extension_list_t {
    using container_type = boost::container::small_vector<extension_t, 2>;

    container_type extensions;


    bool has(boost::string_view name) const {
        return static_cast<bool>(get(name));
    }

    boost::optional<const extension_t &> get(boost::string_view name) const;
};


inline bool operator==(const extension_t &one, const extension_t &another) {
    return one.equals(another.name);
}


inline bool operator!=(const extension_t &one, const extension_t &another) {
    return !(one == another);
}


inline bool operator==(const extension_t &one, boost::string_view another) {
    return one.equals(another);
}


inline bool operator!=(const extension_t &one, boost::string_view another) {
    return !(one == another);
}


inline bool operator==(boost::string_view one, const extension_t &another) {
    return another.equals(one);
}


inline bool operator!=(boost::string_view one, const extension_t &another) {
    return !(one == another);
}


namespace detail {

bool parse_extension_list(boost::string_view data, extension_list_t &result);

} // namespace detail


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
