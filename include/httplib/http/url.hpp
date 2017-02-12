#pragma once

#include <httplib/detail/common.hpp>

#include <boost/optional.hpp>

#include <string>


HTTPLIB_OPEN_NAMESPACE


struct url_t {
    std::string schema;
    std::string user_info;
    std::string host;
    boost::optional<uint16_t> port;
    std::string path;
    std::string query;
    std::string fragment;
};


HTTPLIB_CLOSE_NAMESPACE
