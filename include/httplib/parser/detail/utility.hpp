#pragma once

#include <httplib/detail/common.hpp>

#include <boost/optional.hpp>
#include <boost/system/error_code.hpp>
#include <boost/utility/string_view.hpp>

#include <string>


HTTPLIB_OPEN_NAMESPACE


namespace detail {

// tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA
bool is_tchar(char ch);

// qdtext = HTAB / SP /%x21 / %x23-5B / %x5D-7E / %x80-FF
bool is_qdtext(char ch);

// HTAB / SP / %x21-7E (VCHAR) / %x80-FF
bool is_quoted_pair_char(char ch);

// %x20 (SP) / %x09 (HTAB)
bool is_whitespace(char ch);

// %x41-5A / %x61-7A   ; A-Z / a-z
bool is_alpha(char ch);

// %x30-39  ; 0-9
bool is_digit(char ch);

boost::optional<std::string> parse_token(boost::string_view &data);
boost::optional<std::string> parse_quoted_string(boost::string_view &data);

void skip_optional_whitespaces(boost::string_view &data);

} // namespace detail


HTTPLIB_CLOSE_NAMESPACE
