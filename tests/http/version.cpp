#include <catch.hpp>

#include <httplib/http/version.hpp>

#include <sstream>


TEST_CASE("http_version_t default constructor", "[http_version_t]") {
    const httplib::http_version_t version;

    REQUIRE(version.major == 0);
    REQUIRE(version.minor == 0);
}


TEST_CASE("http_version_t constructor", "[http_version_t]") {
    const httplib::http_version_t version {12, 34};

    REQUIRE(version.major == 12);
    REQUIRE(version.minor == 34);
}


TEST_CASE("http_version_t equality operator", "[http_version_t]") {
    const httplib::http_version_t version1 {12, 34};
    const httplib::http_version_t version2 {12, 34};
    const httplib::http_version_t version3 {12, 35};
    const httplib::http_version_t version4 {11, 34};

    REQUIRE(version1 == version2);
    REQUIRE(!(version1 == version3));
    REQUIRE(!(version3 == version1));
    REQUIRE(!(version1 == version4));
    REQUIRE(!(version4 == version1));
    REQUIRE(!(version3 == version4));
    REQUIRE(!(version4 == version3));
    REQUIRE(httplib::http_version_t() == httplib::http_version_t(0, 0));
}


TEST_CASE("http_version_t inequality operator", "[http_version_t]") {
    const httplib::http_version_t version1 {12, 34};
    const httplib::http_version_t version2 {12, 34};
    const httplib::http_version_t version3 {12, 35};
    const httplib::http_version_t version4 {11, 34};

    REQUIRE(!(version1 != version2));
    REQUIRE(version1 != version3);
    REQUIRE(version3 != version1);
    REQUIRE(version1 != version4);
    REQUIRE(version4 != version1);
    REQUIRE(version3 != version4);
    REQUIRE(version4 != version3);
    REQUIRE(!(httplib::http_version_t() != httplib::http_version_t(0, 0)));
}


TEST_CASE("http_version_t ordering operators", "[http_version_t]") {
    SECTION("equal values") {
        const httplib::http_version_t version1 {12, 34};
        const httplib::http_version_t version2 {12, 34};

        REQUIRE(!(version1 < version2));
        REQUIRE(!(version1 > version2));
        REQUIRE(version1 <= version2);
        REQUIRE(version1 >= version2);
    }

    SECTION("different minor versions") {
        const httplib::http_version_t version1 {12, 34};
        const httplib::http_version_t version2 {12, 35};

        REQUIRE(version1 < version2);
        REQUIRE(!(version2 < version1));
        REQUIRE(!(version1 > version2));
        REQUIRE(version2 > version1);
        REQUIRE(version1 <= version2);
        REQUIRE(!(version2 <= version1));
        REQUIRE(!(version1 >= version2));
        REQUIRE(version2 >= version1);
    }

    SECTION("different major versions") {
        const httplib::http_version_t version1 {11, 34};
        const httplib::http_version_t version2 {12, 34};

        REQUIRE(version1 < version2);
        REQUIRE(!(version2 < version1));
        REQUIRE(!(version1 > version2));
        REQUIRE(version2 > version1);
        REQUIRE(version1 <= version2);
        REQUIRE(!(version2 <= version1));
        REQUIRE(!(version1 >= version2));
        REQUIRE(version2 >= version1);
    }

    SECTION("minors and majors compare differently") {
        const httplib::http_version_t version1 {11, 35};
        const httplib::http_version_t version2 {12, 34};

        REQUIRE(version1 < version2);
        REQUIRE(!(version2 < version1));
        REQUIRE(!(version1 > version2));
        REQUIRE(version2 > version1);
        REQUIRE(version1 <= version2);
        REQUIRE(!(version2 <= version1));
        REQUIRE(!(version1 >= version2));
        REQUIRE(version2 >= version1);
    }

    SECTION("minors and majors compare the same") {
        const httplib::http_version_t version1 {11, 34};
        const httplib::http_version_t version2 {12, 35};

        REQUIRE(version1 < version2);
        REQUIRE(!(version2 < version1));
        REQUIRE(!(version1 > version2));
        REQUIRE(version2 > version1);
        REQUIRE(version1 <= version2);
        REQUIRE(!(version2 <= version1));
        REQUIRE(!(version1 >= version2));
        REQUIRE(version2 >= version1);
    }

    SECTION("one minor is less than the other major") {
        const httplib::http_version_t version1 {11, 34};
        const httplib::http_version_t version2 {12, 10};

        REQUIRE(version1 < version2);
        REQUIRE(!(version2 < version1));
        REQUIRE(!(version1 > version2));
        REQUIRE(version2 > version1);
        REQUIRE(version1 <= version2);
        REQUIRE(!(version2 <= version1));
        REQUIRE(!(version1 >= version2));
        REQUIRE(version2 >= version1);
    }
}


TEST_CASE("http_version_t output operator", "[http_version_t]") {
    const httplib::http_version_t version {12, 34};

    std::ostringstream stream;
    stream << version;

    REQUIRE(stream);
    REQUIRE(stream.str() == "12.34");
}
