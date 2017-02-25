#include <httplib/http/request.hpp>


HTTPLIB_OPEN_NAMESPACE


std::ostream &operator<<(std::ostream &stream, const http_request_t &request) {
    stream << request.method << " " << request.target
           << " HTTP/" << request.version.major << "." << request.version.minor << "\r\n"
           << request.headers << "\r\n";

    return stream;
}


HTTPLIB_CLOSE_NAMESPACE
