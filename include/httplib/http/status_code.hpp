#pragma once

#include <httplib/detail/common.hpp>

#include <boost/utility/string_view.hpp>

#include <string>


HTTPLIB_OPEN_NAMESPACE


class status_code_t {
public:
    status_code_t(unsigned int code, boost::string_view description) :
        m_code(code),
        m_description(description.to_string())
    { }

    unsigned int code() const {
        return m_code;
    }

    boost::string_view description() const {
        return m_description;
    }

private:
    unsigned int m_code;
    std::string m_description;
};


extern status_code_t STATUS_100_CONTINUE;
extern status_code_t STATUS_101_SWITCHING_PROTOCOLS;
extern status_code_t STATUS_102_PROCESSING;
extern status_code_t STATUS_200_OK;
extern status_code_t STATUS_201_CREATED;
extern status_code_t STATUS_202_ACCEPTED;
extern status_code_t STATUS_203_NON_AUTHORITATIVE_INFORMATION;
extern status_code_t STATUS_204_NO_CONTENT;
extern status_code_t STATUS_205_RESET_CONTENT;
extern status_code_t STATUS_206_PARTIAL_CONTENT;
extern status_code_t STATUS_207_MULTI_STATUS;
extern status_code_t STATUS_208_ALREADY_REPORTED;
extern status_code_t STATUS_226_IM_USED;
extern status_code_t STATUS_300_MULTIPLE_CHOICES;
extern status_code_t STATUS_301_MOVED_PERMANENTLY;
extern status_code_t STATUS_302_FOUND;
extern status_code_t STATUS_303_SEE_OTHER;
extern status_code_t STATUS_304_NOT_MODIFIED;
extern status_code_t STATUS_305_USE_PROXY;
extern status_code_t STATUS_307_TEMPORARY_REDIRECT;
extern status_code_t STATUS_308_PERMANENT_REDIRECT;
extern status_code_t STATUS_400_BAD_REQUEST;
extern status_code_t STATUS_401_UNAUTHORIZED;
extern status_code_t STATUS_402_PAYMENT_REQUIRED;
extern status_code_t STATUS_403_FORBIDDEN;
extern status_code_t STATUS_404_NOT_FOUND;
extern status_code_t STATUS_405_METHOD_NOT_ALLOWED;
extern status_code_t STATUS_406_NOT_ACCEPTABLE;
extern status_code_t STATUS_407_PROXY_AUTHENTICATION_REQUIRED;
extern status_code_t STATUS_408_REQUEST_TIMEOUT;
extern status_code_t STATUS_409_CONFLICT;
extern status_code_t STATUS_410_GONE;
extern status_code_t STATUS_411_LENGTH_REQUIRED;
extern status_code_t STATUS_412_PRECONDITION_FAILED;
extern status_code_t STATUS_413_PAYLOAD_TOO_LARGE;
extern status_code_t STATUS_414_URI_TOO_LONG;
extern status_code_t STATUS_415_UNSUPPORTED_MEDIA_TYPE;
extern status_code_t STATUS_416_RANGE_NOT_SATISFIABLE;
extern status_code_t STATUS_417_EXPECTATION_FAILED;
extern status_code_t STATUS_421_MISDIRECTED_REQUEST;
extern status_code_t STATUS_422_UNPROCESSABLE_ENTITY;
extern status_code_t STATUS_423_LOCKED;
extern status_code_t STATUS_424_FAILED_DEPENDENCY;
extern status_code_t STATUS_426_UPGRADE_REQUIRED;
extern status_code_t STATUS_428_PRECONDITION_REQUIRED;
extern status_code_t STATUS_429_TOO_MANY_REQUESTS;
extern status_code_t STATUS_431_REQUEST_HEADER_FIELDS_TOO_LARGE;
extern status_code_t STATUS_451_UNAVAILABLE_FOR_LEGAL_REASONS;
extern status_code_t STATUS_500_INTERNAL_SERVER_ERROR;
extern status_code_t STATUS_501_NOT_IMPLEMENTED;
extern status_code_t STATUS_502_BAD_GATEWAY;
extern status_code_t STATUS_503_SERVICE_UNAVAILABLE;
extern status_code_t STATUS_504_GATEWAY_TIMEOUT;
extern status_code_t STATUS_505_HTTP_VERSION_NOT_SUPPORTED;
extern status_code_t STATUS_506_VARIANT_ALSO_NEGOTIATES;
extern status_code_t STATUS_507_INSUFFICIENT_STORAGE;
extern status_code_t STATUS_508_LOOP_DETECTED;
extern status_code_t STATUS_510_NOT_EXTENDED;
extern status_code_t STATUS_511_NETWORK_AUTHENTICATION_REQUIRED;


HTTPLIB_CLOSE_NAMESPACE
