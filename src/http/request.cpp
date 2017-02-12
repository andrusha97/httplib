#include <httplib/http/request.hpp>


std::ostream &HTTPLIB_NAMESPACE::operator<<(std::ostream &stream, const http_request_t &request) {
    stream << request.method << " " << request.url
           << " HTTP/" << request.version.major << "." << request.version.minor << "\r\n"
           << request.headers << "\r\n";

    return stream;
}
