#include <catch.hpp>

#include <httplib/http/message_properties.hpp>

#include <vector>


namespace {


struct test_case_t {
    httplib::http_headers_t headers;
    bool expect_success;
    httplib::body_size_t::type_t expected_type;
    httplib::content_length_int_t expected_content_length;
};


std::vector<test_case_t> common_tests_cases = {
    { // Detects content-length.
        {
            {"Content-Length", {"123"}}
        },
        true,
        httplib::body_size_t::type_t::content_length,
        123
    },
    { // Rejects multiple headers with the same value.
        {
            {"Content-Length", {"123", "123"}}
        },
        false,
        httplib::body_size_t::type_t::content_length,
        345678
    },
    { // Rejects multiple headers with differenet values.
        {
            {"Content-Length", {"123", "456"}}
        },
        false,
        httplib::body_size_t::type_t::content_length,
        345678
    },
    { // Rejects multiple headers, one of which is empty.
        {
            {"Content-Length", {"123", ""}}
        },
        false,
        httplib::body_size_t::type_t::content_length,
        345678
    },
    { // Rejects content length value which is not a number.
        {
            {"Content-Length", {"abc"}}
        },
        false,
        httplib::body_size_t::type_t::content_length,
        345678
    },
    { // Rejects content length value which is a negative number.
        {
            {"Content-Length", {"-10"}}
        },
        false,
        httplib::body_size_t::type_t::content_length,
        345678
    },
    { // Rejects content length value which is a floating point number.
        {
            {"Content-Length", {"10.1"}}
        },
        false,
        httplib::body_size_t::type_t::content_length,
        345678
    },
    { // Rejects content length value which is a number with exponent.
        {
            {"Content-Length", {"10e3"}}
        },
        false,
        httplib::body_size_t::type_t::content_length,
        345678
    },
    { // Rejects too large numbers.
        {
            {"Content-Length", {"18446744073709551616"}}
        },
        false,
        httplib::body_size_t::type_t::content_length,
        345678
    },
    { // Rejects empty content length.
        {
            {"Content-Length", {""}}
        },
        false,
        httplib::body_size_t::type_t::content_length,
        345678
    }
};


std::vector<test_case_t> http_1_1_tests_cases = {
    { // Prefers transfer encoding.
        {
            {"Content-Length", {"123"}},
            {"Transfer-Encoding", {"chunked"}}
        },
        true,
        httplib::body_size_t::type_t::transfer_encoding,
        3456
    },
    { // Doesn't require content length in presense of transfer encoding.
        {
            {"Transfer-Encoding", {"chunked"}}
        },
        true,
        httplib::body_size_t::type_t::transfer_encoding,
        3456
    },
    { // Considers only presense of transfer encoding, but doesn't parse it.
        {
            {"Transfer-Encoding", {"dvsdv3e4@#$%^", "@#$%^&*(kjn)d*!"}}
        },
        true,
        httplib::body_size_t::type_t::transfer_encoding,
        3456
    }
};


std::vector<test_case_t> http_1_0_tests_cases = {
    { // Doesn't support transfer encoding for 1.0.
        {
            {"Transfer-Encoding", {"chunked"}},
            {"Content-Length", {"98"}}
        },
        true,
        httplib::body_size_t::type_t::content_length,
        98
    }
};


void check_test_case_result(const test_case_t &test_case, const boost::optional<httplib::body_size_t> &result) {
    REQUIRE(static_cast<bool>(result) == test_case.expect_success);

    if (test_case.expect_success) {
        REQUIRE(result->type == test_case.expected_type);

        if (test_case.expected_type == httplib::body_size_t::type_t::content_length) {
            REQUIRE(result->content_length == test_case.expected_content_length);
        }
    }
}


} // namespace


TEST_CASE("body_size for a request returns 0 when content length is not specified", "[body_size]") {
    for (auto version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
        CAPTURE(version);

        const httplib::http_request_t request {
            "GET",
            "/",
            version,
            {
                {"abc", {"123"}}
            }
        };

        auto body_size = httplib::body_size(request);

        REQUIRE(body_size);
        REQUIRE(body_size->type == httplib::body_size_t::type_t::content_length);
        REQUIRE(body_size->content_length == 0);
    }
}


TEST_CASE("body_size for requests of any version", "[body_size]") {
    for (auto test_case: common_tests_cases) {
        for (auto version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
            CAPTURE(test_case.headers);
            CAPTURE(version);

            const httplib::http_request_t request {"GET", "/", version, test_case.headers};
            check_test_case_result(test_case, httplib::body_size(request));
        }
    }
}


TEST_CASE("body_size for http 1.0 requests", "[body_size]") {
    std::vector<test_case_t> test_cases = http_1_0_tests_cases;

    test_cases.push_back({
        {
            {"Transfer-Encoding", {"chunked"}}
        },
        true,
        httplib::body_size_t::type_t::content_length,
        0
    });

    for (auto test_case: test_cases) {
        CAPTURE(test_case.headers);

        const httplib::http_request_t request {"GET", "/", {1, 0}, test_case.headers};
        check_test_case_result(test_case, httplib::body_size(request));
    }
}


TEST_CASE("body_size for http 1.1 requests", "[body_size]") {
    for (auto test_case: http_1_1_tests_cases) {
        CAPTURE(test_case.headers);

        const httplib::http_request_t request {"GET", "/", {1, 1}, test_case.headers};
        check_test_case_result(test_case, httplib::body_size(request));
    }
}


TEST_CASE("body_size for a response returns until_eof when content length is not specified", "[body_size]") {
    for (auto version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
        CAPTURE(version);

        const httplib::http_response_t response {
            200,
            "OK",
            version,
            {
                {"abc", {"123"}}
            }
        };

        auto body_size = httplib::body_size(response);

        REQUIRE(body_size);
        REQUIRE(body_size->type == httplib::body_size_t::type_t::until_eof);
    }
}


TEST_CASE("body_size is aware that some replies cannot have a body", "[body_size]") {
    for (auto version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
        for (unsigned int code: {100, 150, 199, 204, 304}) {
            CAPTURE(version);
            CAPTURE(code);

            const httplib::http_response_t response {
                code,
                "OK",
                version,
                {
                    {"abc", {"123"}},
                    {"Transfer-Encoding", {"chunked"}},
                    {"Content-Length", {"100"}}
                }
            };

            auto body_size = httplib::body_size(response);

            REQUIRE(body_size);
            REQUIRE(body_size->type == httplib::body_size_t::type_t::content_length);
            REQUIRE(body_size->content_length == 0);
        }
    }
}


TEST_CASE("body_size for replies of any version", "[body_size]") {
    for (auto test_case: common_tests_cases) {
        for (auto version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
            CAPTURE(test_case.headers);
            CAPTURE(version);

            const httplib::http_response_t response {200, "OK", version, test_case.headers};
            check_test_case_result(test_case, httplib::body_size(response));
        }
    }
}


TEST_CASE("body_size for http 1.0 replies", "[body_size]") {
    std::vector<test_case_t> test_cases = http_1_0_tests_cases;

    test_cases.push_back({
        {
            {"Transfer-Encoding", {"chunked"}}
        },
        true,
        httplib::body_size_t::type_t::until_eof,
        456890
    });

    for (auto test_case: test_cases) {
        CAPTURE(test_case.headers);

        const httplib::http_response_t response {200, "OK", {1, 0}, test_case.headers};
        check_test_case_result(test_case, httplib::body_size(response));
    }
}


TEST_CASE("body_size for http 1.1 replies", "[body_size]") {
    for (auto test_case: http_1_1_tests_cases) {
        CAPTURE(test_case.headers);

        const httplib::http_response_t response {200, "OK", {1, 1}, test_case.headers};
        check_test_case_result(test_case, httplib::body_size(response));
    }
}


TEST_CASE("body_size is aware that some replies cannot have a body regardless of the request", "[body_size]") {
    for (auto version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
        for (unsigned int code: {100, 150, 199, 204, 304}) {
            for (auto request_version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
                for (auto method: {"GET", "POST", "OLOLO", "PUT"}) {
                    CAPTURE(version);
                    CAPTURE(code);
                    CAPTURE(request_version);
                    CAPTURE(method);

                    const httplib::http_request_t request {
                        method,
                        "/",
                        request_version,
                        {}
                    };

                    const httplib::http_response_t response {
                        code,
                        "OK",
                        version,
                        {
                            {"abc", {"123"}},
                            {"Transfer-Encoding", {"chunked"}},
                            {"Content-Length", {"100"}}
                        }
                    };

                    auto body_size = httplib::body_size(response, request);

                    REQUIRE(body_size);
                    REQUIRE(body_size->type == httplib::body_size_t::type_t::content_length);
                    REQUIRE(body_size->content_length == 0);
                }
            }
        }
    }
}


TEST_CASE("body_size is aware that replies on HEAD cannot have a body", "[body_size]") {
    for (auto version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
        for (unsigned int code: {100, 150, 199, 200, 204, 215, 300, 304, 400, 404, 500, 503}) {
            for (auto request_version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
                CAPTURE(version);
                CAPTURE(code);
                CAPTURE(request_version);

                const httplib::http_request_t request {"HEAD", "/", request_version, {}};

                const httplib::http_response_t response {
                    code,
                    "OK",
                    version,
                    {
                        {"abc", {"123"}},
                        {"Transfer-Encoding", {"chunked"}},
                        {"Content-Length", {"100"}}
                    }
                };

                auto body_size = httplib::body_size(response, request);

                REQUIRE(body_size);
                REQUIRE(body_size->type == httplib::body_size_t::type_t::content_length);
                REQUIRE(body_size->content_length == 0);
            }
        }
    }
}


TEST_CASE("body_size is aware that successful replies on CONNECT cannot have a body", "[body_size]") {
    for (auto version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
        for (unsigned int code: {200, 204, 215, 299}) {
            for (auto request_version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
                CAPTURE(version);
                CAPTURE(code);
                CAPTURE(request_version);

                const httplib::http_request_t request {"CONNECT", "/", request_version, {}};

                const httplib::http_response_t response {
                    code,
                    "OK",
                    version,
                    {
                        {"abc", {"123"}},
                        {"Transfer-Encoding", {"chunked"}},
                        {"Content-Length", {"100"}}
                    }
                };

                auto body_size = httplib::body_size(response, request);

                REQUIRE(body_size);
                REQUIRE(body_size->type == httplib::body_size_t::type_t::content_length);
                REQUIRE(body_size->content_length == 0);
            }
        }
    }
}


TEST_CASE("body_size is aware that non-200 replies on CONNECT can have a body", "[body_size]") {
    std::vector<test_case_t> response_1_0_test_cases = http_1_0_tests_cases;

    response_1_0_test_cases.push_back({
        {
            {"Transfer-Encoding", {"chunked"}}
        },
        true,
        httplib::body_size_t::type_t::until_eof,
        456890
    });

    for (auto request_version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
        CAPTURE(request_version);

        const httplib::http_request_t request {"CONNECT", "/", request_version, {}};

        for (unsigned int code: {300, 301, 350, 399, 400, 404, 499, 500, 503}) {
            CAPTURE(code);

            for (auto test_case: common_tests_cases) {
                for (auto version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
                    CAPTURE(test_case.headers);
                    CAPTURE(version);

                    const httplib::http_response_t response {code, "FYVA", version, test_case.headers};
                    check_test_case_result(test_case, httplib::body_size(response, request));
                }
            }

            for (auto test_case: response_1_0_test_cases) {
                CAPTURE(test_case.headers);

                const httplib::http_response_t response {code, "FYVA", {1, 0}, test_case.headers};
                check_test_case_result(test_case, httplib::body_size(response, request));
            }

            for (auto test_case: http_1_1_tests_cases) {
                CAPTURE(test_case.headers);

                const httplib::http_response_t response {code, "FYVA", {1, 1}, test_case.headers};
                check_test_case_result(test_case, httplib::body_size(response, request));
            }
        }
    }
}


TEST_CASE("body_size for replies of any version with requests", "[body_size]") {
    for (auto request_version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
        for (auto test_case: common_tests_cases) {
            for (auto version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
                CAPTURE(test_case.headers);
                CAPTURE(version);
                CAPTURE(request_version);

                const httplib::http_request_t request {"GET", "/", request_version, {}};
                const httplib::http_response_t response {200, "OK", version, test_case.headers};
                check_test_case_result(test_case, httplib::body_size(response, request));
            }
        }
    }
}


TEST_CASE("body_size for http 1.0 replies with requests", "[body_size]") {
    std::vector<test_case_t> test_cases = http_1_0_tests_cases;

    test_cases.push_back({
        {
            {"Transfer-Encoding", {"chunked"}}
        },
        true,
        httplib::body_size_t::type_t::until_eof,
        456890
    });

    for (auto request_version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
        for (auto test_case: test_cases) {
            CAPTURE(test_case.headers);
            CAPTURE(request_version);

            const httplib::http_request_t request {"GET", "/", request_version, {}};
            const httplib::http_response_t response {200, "OK", {1, 0}, test_case.headers};
            check_test_case_result(test_case, httplib::body_size(response, request));
        }
    }
}


TEST_CASE("body_size for http 1.1 replies with requests", "[body_size]") {
    for (auto request_version: {httplib::http_version_t {1, 0}, httplib::http_version_t {1, 1}}) {
        for (auto test_case: http_1_1_tests_cases) {
            CAPTURE(test_case.headers);
            CAPTURE(request_version);

            const httplib::http_request_t request {"GET", "/", request_version, {}};
            const httplib::http_response_t response {200, "OK", {1, 1}, test_case.headers};
            check_test_case_result(test_case, httplib::body_size(response, request));
        }
    }
}
