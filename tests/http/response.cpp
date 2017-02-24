#include <catch.hpp>

#include <httplib/http/response.hpp>

#include <sstream>


TEST_CASE("response's output operator outputs", "[http_response_t]") {
    const httplib::http_response_t response {
        1337,
        "ABC DE F",
        {12, 3},
        {
            {"Content-Length", {"10"}},
            {"Content-Type", {"application/json"}},
            {"xxx", {"yyy", "zzz"}}
        }
    };

    std::ostringstream stream;
    stream << response;

    REQUIRE(static_cast<bool>(stream));

    std::string expected =
        "HTTP/12.3 1337 ABC DE F\r\n"
        "Content-Length: 10\r\n"
        "Content-Type: application/json\r\n"
        "xxx: yyy\r\n"
        "xxx: zzz\r\n"
        "\r\n";

    REQUIRE(stream.str() == expected);
}
