#pragma once

#include <httplib/detail/common.hpp>

#include <boost/optional.hpp>
#include <boost/system/error_code.hpp>
#include <boost/utility/string_view.hpp>

#include <string>


HTTPLIB_OPEN_NAMESPACE

namespace detail {

bool is_tchar(char ch);
bool is_qdtext(char ch);
bool is_quoted_pair_char(char ch);
bool is_whitespace(char ch);

boost::optional<std::string> parse_token(boost::string_view &data);
boost::optional<std::string> parse_quoted_string(boost::string_view &data);

void skip_optional_whitespaces(boost::string_view &data);

} // namespace detail

HTTPLIB_CLOSE_NAMESPACE
