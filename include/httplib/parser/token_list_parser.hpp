#pragma once

#include <httplib/detail/common.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>

#include <cstdlib>
#include <string>


HTTPLIB_OPEN_NAMESPACE

class token_list_t;


namespace detail {

class token_access_t;
class token_list_access_t;

bool parse_token_list(boost::string_view data, token_list_t &result);

} // namespace detail


class token_t {
    friend class detail::token_access_t;

public:
    boost::string_view value() const;

    // Compare case-insensitively.
    bool equals(boost::string_view value) const;
    bool operator==(boost::string_view value) const;
    bool operator!=(boost::string_view value) const;

private:
    std::string m_value;
};


// *( "," OWS ) token *( OWS "," [ OWS token ] )
class token_list_t {
    friend class detail::token_list_access_t;

    using container_type = boost::container::small_vector<token_t, 2>;

public:
    using const_iterator = container_type::const_iterator;

public:
    std::size_t size() const;

    const_iterator begin() const;
    const_iterator end() const;

    bool has(boost::string_view token) const;

private:
    container_type m_tokens;
};


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
