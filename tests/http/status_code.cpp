#include <catch.hpp>

#include <httplib/http/status_code.hpp>


TEST_CASE("status_code constructor", "[status_code_t]") {
    const httplib::status_code_t status(123, "ololo lo lo");

    REQUIRE(status.code() == 123);
    REQUIRE(status.description() == "ololo lo lo");
}


TEST_CASE("status_code is copyable", "[status_code_t]") {
    const httplib::status_code_t status1(123, "ololo lo lo");
    const httplib::status_code_t status2 = status1;

    REQUIRE(status1.code() == 123);
    REQUIRE(status1.description() == "ololo lo lo");

    REQUIRE(status2.code() == 123);
    REQUIRE(status2.description() == "ololo lo lo");
}


TEST_CASE("status_code is assignable", "[status_code_t]") {
    httplib::status_code_t status1(123, "ololo lo lo");

    REQUIRE(status1.code() == 123);
    REQUIRE(status1.description() == "ololo lo lo");

    const httplib::status_code_t status2(456, "qwerty asdf");

    status1 = status2;

    REQUIRE(status1.code() == 456);
    REQUIRE(status1.description() == "qwerty asdf");

    REQUIRE(status2.code() == 456);
    REQUIRE(status2.description() == "qwerty asdf");
}
