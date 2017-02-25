#pragma once

#include <httplib/detail/common.hpp>

#include <iostream>


HTTPLIB_OPEN_NAMESPACE


struct http_version_t {
    unsigned int major;
    unsigned int minor;

    http_version_t() :
        http_version_t(0, 0)
    { }

    http_version_t(unsigned int major, unsigned int minor) :
        major {major},
        minor {minor}
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


inline std::ostream &operator<<(std::ostream &stream, const http_version_t &version) {
    stream << version.major << "." << version.minor;
    return stream;
}


HTTPLIB_CLOSE_NAMESPACE
