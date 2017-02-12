#pragma once

#include <httplib/detail/common.hpp>


HTTPLIB_OPEN_NAMESPACE

struct http_version_t {
    unsigned int major;
    unsigned int minor;

    http_version_t() :
        http_version_t(0, 0)
    { }

    http_version_t(unsigned int major, unsigned int minor) :
        major(major),
        minor(minor)
    { }
};

inline bool operator==(const http_version_t &one, const http_version_t &another) {
    return one.major == another.major &&
           one.minor == another.minor;
}

inline bool operator<(const http_version_t &one, const http_version_t &another) {
    return one.major < another.major ||
           (one.major == another.major && one.minor < another.minor);
}

inline bool operator!=(const http_version_t &one, const http_version_t &another) {
    return !(one == another);
}

inline bool operator>(const http_version_t &one, const http_version_t &another) {
    return another < one;
}

inline bool operator<=(const http_version_t &one, const http_version_t &another) {
    return !(one > another);
}

inline bool operator>=(const http_version_t &one, const http_version_t &another) {
    return !(one < another);
}

HTTPLIB_CLOSE_NAMESPACE
