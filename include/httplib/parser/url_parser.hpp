#pragma once

#include <httplib/detail/common.hpp>
#include <httplib/http/url.hpp>

#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>


HTTPLIB_OPEN_NAMESPACE


boost::optional<url_t> parse_url(boost::string_view data);


HTTPLIB_CLOSE_NAMESPACE
