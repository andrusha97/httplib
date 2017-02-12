#pragma once

#define HTTPLIB_ABI_VERSION v1

#define HTTPLIB_NAMESPACE httplib::HTTPLIB_ABI_VERSION

#define HTTPLIB_OPEN_NAMESPACE namespace httplib { inline namespace HTTPLIB_ABI_VERSION {
#define HTTPLIB_CLOSE_NAMESPACE }}
