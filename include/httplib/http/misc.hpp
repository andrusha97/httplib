#pragma once

#include <httplib/detail/common.hpp>

#include <cstdint>
#include <type_traits>


HTTPLIB_OPEN_NAMESPACE

using content_length_int_t = std::conditional_t<
    (sizeof(std::size_t) >= sizeof(std::uint64_t)),
    std::size_t,
    std::uint64_t
>;

HTTPLIB_CLOSE_NAMESPACE
