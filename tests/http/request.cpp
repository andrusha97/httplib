#include <catch.hpp>

#include <httplib/http/request.hpp>

#include <sstream>


TEST_CASE("request's output operator outputs", "[http_request_t]") {
    httplib::http_request_t request {
        "MyMETHOD",
        "/url/path?query&arg=value#frag",
        {13, 37},
        {
            {"Content-Length", {"10"}},
            {"Content-Type", {"application/json"}},
            {"Home", {"localhost"}},
            {"xxx", {"yyy", "zzz"}}
        }
    };

    std::ostringstream stream;
    stream << request;

    REQUIRE(static_cast<bool>(stream));

    std::string expected =
        "MyMETHOD /url/path?query&arg=value#frag HTTP/13.37\r\n"
        "Home: localhost\r\n"
        "Content-Length: 10\r\n"
        "Content-Type: application/json\r\n"
        "xxx: yyy\r\n"
        "xxx: zzz\r\n"
        "\r\n";

    REQUIRE(stream.str() == expected);
}
