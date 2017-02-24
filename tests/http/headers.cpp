#include <catch.hpp>

#include <httplib/http/headers.hpp>


TEST_CASE("default-constructed headers", "[http_headers_t]") {
    const httplib::http_headers_t headers;

    REQUIRE(headers.empty());
    REQUIRE(headers.size() == 0);
    REQUIRE(headers.begin() == headers.end());
    REQUIRE(headers.rbegin() == headers.rend());
    REQUIRE(headers.find("") == headers.end());
    REQUIRE(!headers.has(""));
    REQUIRE(!headers.get_header(""));
    REQUIRE(!headers.get_header_values(""));
}
