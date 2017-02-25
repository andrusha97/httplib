#include <catch.hpp>

#include <httplib/http/message_properties.hpp>

#include <sstream>
#include <vector>


namespace {


struct test_case_t {
    httplib::http_version_t version;
    httplib::http_headers_t headers;

    boost::optional<httplib::connection_status_t> expected_result;
};


std::vector<test_case_t> tests_cases = {
    { // For 1.0 close by default.
        {1, 0},
        { },
        httplib::connection_status_t::close
    },
    { // For 1.1 keep-alive by default.
        {1, 1},
        { },
        httplib::connection_status_t::keep_alive
    },
    { // Keep-alive for 1.0.
        {1, 0},
        {
            {"Connection", {"abcde, frg, dds", "fvfv, keep-alive,dfvdsfv", "ddvdfv"}}
        },
        httplib::connection_status_t::keep_alive
    },
    { // Close overrides keep-alive in 1.0.
        {1, 0},
        {
            {"Connection", {"abcde, frg, dds", "fvfv,close, keep-alive,dfvdsfv", "ddvdfv"}}
        },
        httplib::connection_status_t::close
    },
    { // Close overrides keep-alive in 1.0 again.
        {1, 0},
        {
            {"Connection", {"abcde, frg, dds", "fvfv, keep-alive,dfvdsfv,close", "ddvdfv"}}
        },
        httplib::connection_status_t::close
    },
    { // Close in 1.1.
        {1, 1},
        {
            {"Connection", {"abcde, frg, dds", "fvfv,dfvdsfv,close, dvdf", "ddvdfv"}}
        },
        httplib::connection_status_t::close
    },
    { // Tokens in connection header are case-insensitive.
        {1, 0},
        {
            {"Connection", {"keep-alive"}}
        },
        httplib::connection_status_t::keep_alive
    },
    { // Tokens in connection header are case-insensitive.
        {1, 0},
        {
            {"Connection", {"KeEP-AliVe"}}
        },
        httplib::connection_status_t::keep_alive
    },
    { // Tokens in connection header are case-insensitive.
        {1, 1},
        {
            {"Connection", {"cLoSe"}}
        },
        httplib::connection_status_t::close
    },
    { // connection_status rejects bad connection headers.
        {1, 1},
        {
            {"Connection", {"@"}}
        },
        boost::none
    },
    { // connection_status rejects bad connection headers.
        {1, 1},
        {
            {"Connection", {"close,@"}}
        },
        boost::none
    },
    { // connection_status rejects bad connection headers.
        {1, 1},
        {
            {"Connection", {"@,close"}}
        },
        boost::none
    },
    { // connection_status rejects bad connection headers.
        {1, 1},
        {
            {"Connection", {"@", "close"}}
        },
        boost::none
    },
    { // connection_status rejects bad connection headers.
        {1, 1},
        {
            {"Connection", {"close", "@"}}
        },
        boost::none
    },
    { // connection_status rejects bad connection headers.
        {1, 1},
        {
            {"Connection", {"close;"}}
        },
        boost::none
    },
    { // connection_status rejects bad connection headers.
        {1, 1},
        {
            {"Connection", {";close"}}
        },
        boost::none
    },
    { // connection_status rejects bad connection headers.
        {1, 1},
        {
            {"Connection", {""}}
        },
        boost::none
    }
};


} // namespace


namespace Catch {

template<>
struct StringMaker<boost::optional<httplib::connection_status_t>> {
    static std::string convert(const boost::optional<httplib::connection_status_t> &value) {
        std::ostringstream output;

        if (value) {
            output << int(*value);
        } else {
            output << "none";
        }

        return output.str();
    }
};

} // namespace Catch



TEST_CASE("connection_status of a request", "[connection_status]") {
    for (auto test_case: tests_cases) {
        CAPTURE(test_case.version);
        CAPTURE(test_case.headers);

        const httplib::http_request_t request {"GET", "/", test_case.version, test_case.headers};
        REQUIRE(httplib::connection_status(request) == test_case.expected_result);
    }
}


TEST_CASE("connection_status of a response", "[connection_status]") {
    for (auto test_case: tests_cases) {
        CAPTURE(test_case.version);
        CAPTURE(test_case.headers);

        const httplib::http_response_t response {200, "OK", test_case.version, test_case.headers};
        REQUIRE(httplib::connection_status(response) == test_case.expected_result);
    }
}
