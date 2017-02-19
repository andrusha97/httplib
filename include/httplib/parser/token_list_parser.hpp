#pragma once

#include <httplib/detail/common.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>

#include <cstdlib>
#include <string>


HTTPLIB_OPEN_NAMESPACE


struct token_t {
    std::string value;

    // Compare case-insensitively.
    bool equals(boost::string_view value) const;
};


// *( "," OWS ) token *( OWS "," [ OWS token ] )
struct token_list_t {
    using container_type = boost::container::small_vector<token_t, 2>;

    container_type tokens;

    bool has(boost::string_view token) const;
};


inline bool operator==(const token_t &one, const token_t &another) {
    return one.equals(another.value);
}


inline bool operator!=(const token_t &one, const token_t &another) {
    return !(one == another);
}


inline bool operator==(const token_t &one, boost::string_view another) {
    return one.equals(another);
}


inline bool operator!=(const token_t &one, boost::string_view another) {
    return !(one == another);
}


inline bool operator==(boost::string_view one, const token_t &another) {
    return another.equals(one);
}


inline bool operator!=(boost::string_view one, const token_t &another) {
    return !(one == another);
}


namespace detail {

bool parse_token_list(boost::string_view data, token_list_t &result);

} // namespace detail


// Parse a list from a header value.
boost::optional<token_list_t> parse_token_list(boost::string_view data);


// Parse a list from a sequence of header values.
template<class It>
boost::optional<token_list_t> parse_token_list(It begin, It end) {
    if (begin == end) {
        return boost::none;
    }

    token_list_t result;

    for (; begin != end; ++begin) {
        if (!detail::parse_token_list(*begin, result)) {
            return boost::none;
        }
    }

    return result;
}


HTTPLIB_CLOSE_NAMESPACE
