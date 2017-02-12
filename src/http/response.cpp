#include <httplib/http/response.hpp>


std::ostream &HTTPLIB_NAMESPACE::operator<<(std::ostream &stream, const http_response_t &response) {
    stream << "HTTP/" << response.version.major << "." << response.version.minor << " "
           << response.code << " " << response.reason << "\r\n" << response.headers << "\r\n";

    return stream;
}
