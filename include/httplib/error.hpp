#pragma once

#include <httplib/detail/common.hpp>

#include <boost/system/error_code.hpp>

#include <type_traits>


HTTPLIB_OPEN_NAMESPACE

enum class parser_errc_t {
    too_long_header = 1,
    too_many_headers,
    too_long_url,
    too_long_reason,
    invalid_parser
};

boost::system::error_code make_error_code(parser_errc_t e) noexcept;
boost::system::error_condition make_error_condition(parser_errc_t e) noexcept;

const boost::system::error_category &parser_category() noexcept;
const boost::system::error_category &underlying_parser_category() noexcept;


enum class reader_errc_t {
    eof = 1
};

boost::system::error_code make_error_code(reader_errc_t e) noexcept;
boost::system::error_condition make_error_condition(reader_errc_t e) noexcept;

const boost::system::error_category &reader_category() noexcept;

HTTPLIB_CLOSE_NAMESPACE


namespace boost { namespace system {

template<>
struct is_error_condition_enum<httplib::parser_errc_t> : std::true_type { };

template<>
struct is_error_code_enum<httplib::reader_errc_t> : std::true_type { };

}} // namespace boost::system
