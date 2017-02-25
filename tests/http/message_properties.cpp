#include <catch.hpp>

#include <httplib/http/message_properties.hpp>


TEST_CASE("body_size for a request returns 0 when content length is not specified", "[body_size]") {
    httplib::http_version_t version;

    SECTION("http 1.0") {
        version = {1, 0};
    }

    SECTION("http 1.1") {
        version = {1, 1};
    }

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


TEST_CASE("body_size for a request returns content length", "[body_size]") {
    httplib::http_version_t version;

    SECTION("http 1.0") {
        version = {1, 0};
    }

    SECTION("http 1.1") {
        version = {1, 1};
    }

    const httplib::http_request_t request {
        "GET",
        "/",
        version,
        {
            {"Content-Length", {"123"}}
        }
    };

    auto body_size = httplib::body_size(request);

    REQUIRE(body_size);
    REQUIRE(body_size->type == httplib::body_size_t::type_t::content_length);
    REQUIRE(body_size->content_length == 123);
}


TEST_CASE("body_size for a http 1.1 request prefers transfer encoding", "[body_size]") {
    const httplib::http_request_t request {
        "GET",
        "/",
        {1, 1},
        {
            {"Content-Length", {"123"}},
            {"Transfer-Encoding", {"chunked"}}
        }
    };

    auto body_size = httplib::body_size(request);

    REQUIRE(body_size);
    REQUIRE(body_size->type == httplib::body_size_t::type_t::transfer_encoding);
}


TEST_CASE("body_size for a http 1.1 request doesn't require content length", "[body_size]") {
    const httplib::http_request_t request {
        "GET",
        "/",
        {1, 1},
        {
            {"Transfer-Encoding", {"chunked"}}
        }
    };

    auto body_size = httplib::body_size(request);

    REQUIRE(body_size);
    REQUIRE(body_size->type == httplib::body_size_t::type_t::transfer_encoding);
}


TEST_CASE("body_size for a http 1.1 request doesn't parse the exact transfer encoding", "[body_size]") {
    const httplib::http_request_t request {
        "GET",
        "/",
        {1, 1},
        {
            {"Transfer-Encoding", {"dvsdv3e4@#$%^", "@#$%^&*(kjn)d*!"}}
        }
    };

    auto body_size = httplib::body_size(request);

    REQUIRE(body_size);
    REQUIRE(body_size->type == httplib::body_size_t::type_t::transfer_encoding);
}


TEST_CASE("body_size for a http 1.0 request doesn't support transfer encoding", "[body_size]") {
    SECTION("no content length") {
        const httplib::http_request_t request {
            "GET",
            "/",
            {1, 0},
            {
                {"Transfer-Encoding", {"chunked"}}
            }
        };

        auto body_size = httplib::body_size(request);

        REQUIRE(body_size);
        REQUIRE(body_size->type == httplib::body_size_t::type_t::content_length);
        REQUIRE(body_size->content_length == 0);
    }

    SECTION("with content length") {
        const httplib::http_request_t request {
            "GET",
            "/",
            {1, 0},
            {
                {"Transfer-Encoding", {"chunked"}},
                {"Content-Length", {"98"}}
            }
        };

        auto body_size = httplib::body_size(request);

        REQUIRE(body_size);
        REQUIRE(body_size->type == httplib::body_size_t::type_t::content_length);
        REQUIRE(body_size->content_length == 98);
    }
}


TEST_CASE("body_size for a request rejects multiple content length headers", "[body_size]") {
    httplib::http_headers_t::header_values_t content_length;

    SECTION("same values") {
        content_length = {"123", "123"};
    }

    SECTION("different values") {
        content_length = {"123", "456"};
    }

    SECTION("one empty") {
        content_length = {"123", ""};
    }

    const httplib::http_request_t request {
        "GET",
        "/",
        {1, 1},
        {
            {"Content-Length", content_length}
        }
    };

    auto body_size = httplib::body_size(request);

    REQUIRE(!body_size);
}


TEST_CASE("body_size for a request rejects bad content length values", "[body_size]") {
    SECTION("not a number") {
        const httplib::http_request_t request {
            "GET",
            "/",
            {1, 1},
            {
                {"Content-Length", {"abc"}}
            }
        };

        REQUIRE(!httplib::body_size(request));
    }

    SECTION("negative number") {
        const httplib::http_request_t request {
            "GET",
            "/",
            {1, 1},
            {
                {"Content-Length", {"-10"}}
            }
        };

        REQUIRE(!httplib::body_size(request));
    }

    SECTION("floating point number") {
        const httplib::http_request_t request {
            "GET",
            "/",
            {1, 1},
            {
                {"Content-Length", {"10.1"}}
            }
        };

        REQUIRE(!httplib::body_size(request));
    }

    SECTION("number with exponent") {
        const httplib::http_request_t request {
            "GET",
            "/",
            {1, 1},
            {
                {"Content-Length", {"10e3"}}
            }
        };

        REQUIRE(!httplib::body_size(request));
    }

    SECTION("very long number") {
        const httplib::http_request_t request {
            "GET",
            "/",
            {1, 1},
            {
                {"Content-Length", {"18446744073709551616"}}
            }
        };

        REQUIRE(!httplib::body_size(request));
    }

    SECTION("empty") {
        const httplib::http_request_t request {
            "GET",
            "/",
            {1, 1},
            {
                {"Content-Length", {""}}
            }
        };

        REQUIRE(!httplib::body_size(request));
    }
}
