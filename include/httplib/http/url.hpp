#pragma once

#include <httplib/detail/common.hpp>

#include <boost/optional.hpp>

#include <string>


HTTPLIB_OPEN_NAMESPACE


struct url_t {
    boost::optional<std::string> schema;
    boost::optional<std::string> user_info;
    boost::optional<std::string> host;
    boost::optional<uint16_t> port;
    std::string path;
    boost::optional<std::string> query;
    boost::optional<std::string> fragment;
};


HTTPLIB_CLOSE_NAMESPACE
