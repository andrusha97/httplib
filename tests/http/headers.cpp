#include <catch.hpp>

#include <httplib/http/headers.hpp>

#include <set>
#include <sstream>
#include <vector>


namespace {

void validate_empty_headers(const httplib::http_headers_t &headers) {
    REQUIRE(headers.empty());
    REQUIRE(headers.size() == 0);
    REQUIRE(headers.begin() == headers.end());
    REQUIRE(headers.find("header1") == headers.end());
    REQUIRE(headers.find("") == headers.end());
    REQUIRE(!headers.has("Header2"));
    REQUIRE(!headers.has(""));
    REQUIRE(!headers.get_header("header2"));
    REQUIRE(!headers.get_header(""));
    REQUIRE(!headers.get_header_values("Header1"));
    REQUIRE(!headers.get_header_values(""));
}


using vector_t = std::vector<std::pair<httplib::http_headers_t::header_name_t, httplib::http_headers_t::header_values_t>>;
using set_t = std::set<std::pair<httplib::http_headers_t::header_name_t, httplib::http_headers_t::header_values_t>>;


vector_t non_empty_headers_init = {
    {"Header1", {"Value1", "Value2"}},
    {"header2", {"value3"}},
    {"hEaDeR1", {"Value4"}},
    {"EmptyHeader", {}},
    {"header1", {"Value1"}}
};


set_t non_empty_headers_set {
    {"Header1", {"Value1", "Value2", "Value4", "Value1"}},
    {"header2", {"value3"}}
};


void validate_non_empty_headers(const httplib::http_headers_t &headers) {
    REQUIRE(non_empty_headers_set == set_t(headers.begin(), headers.end()));

    REQUIRE(!headers.empty());
    REQUIRE(headers.size() == 5);
    REQUIRE(headers.begin() != headers.end());
    REQUIRE(headers.find("header1") != headers.end());
    REQUIRE(headers.find("EmptyHeader") == headers.end());
    REQUIRE(headers.has("Header2"));
    REQUIRE(!headers.has("EmptyHeader"));
    REQUIRE(headers.get_header("header2"));
    REQUIRE(headers.get_header_values("Header1"));
    REQUIRE(!headers.get_header_values("EmptyHeader"));
}


std::string headers_to_string(const httplib::http_headers_t &headers) {
    std::ostringstream stream;
    stream << headers;

    REQUIRE(stream);

    return stream.str();
}

} // namespace


TEST_CASE("default-constructed headers", "[http_headers_t]") {
    const httplib::http_headers_t headers;

    validate_empty_headers(headers);
}


TEST_CASE("headers' iterators range constructor", "[http_headers_t]") {
    const httplib::http_headers_t headers(non_empty_headers_init.begin(), non_empty_headers_init.end());
    validate_non_empty_headers(headers);
}


TEST_CASE("headers' initialization with an empty range", "[http_headers_t]") {
    vector_t empty_vector;
    const httplib::http_headers_t headers(empty_vector.begin(), empty_vector.end());
    validate_empty_headers(headers);
}


TEST_CASE("headers' initializer list constructor", "[http_headers_t]") {
    const httplib::http_headers_t headers {
        {"Header1", {"Value1", "Value2"}},
        {"header2", {"value3"}},
        {"hEaDeR1", {"Value4"}},
        {"EmptyHeader", {}},
        {"header1", {"Value1"}}
    };

    validate_non_empty_headers(headers);
}


TEST_CASE("headers' initialization with an empty list", "[http_headers_t]") {
    const httplib::http_headers_t headers = {};
    validate_empty_headers(headers);
}


SCENARIO("headers' copy-constructor copies", "[http_headers_t]") {
    GIVEN("a non-empty headers object") {
        const httplib::http_headers_t headers(non_empty_headers_init.begin(), non_empty_headers_init.end());

        WHEN("we copy-construct another object from it") {
            httplib::http_headers_t copy = headers;

            THEN("they both contain the headers") {
                validate_non_empty_headers(headers);
                validate_non_empty_headers(copy);
            }
        }
    }


    GIVEN("an empty headers object") {
        const httplib::http_headers_t headers;

        WHEN("we copy-construct another object from it") {
            httplib::http_headers_t copy = headers;

            THEN("they're both empty") {
                validate_empty_headers(headers);
                validate_empty_headers(copy);
            }
        }
    }
}


SCENARIO("headers' move-constructor moves", "[http_headers_t]") {
    GIVEN("a non-empty headers object") {
        httplib::http_headers_t headers(non_empty_headers_init.begin(), non_empty_headers_init.end());

        WHEN("we move-construct another object from it") {
            httplib::http_headers_t copy = std::move(headers);

            THEN("new object contains the headers") {
                validate_non_empty_headers(copy);
            }
        }
    }


    GIVEN("an empty headers object") {
        httplib::http_headers_t headers;

        WHEN("we move-construct another object from it") {
            httplib::http_headers_t copy = std::move(headers);

            THEN("new object is empty") {
                validate_empty_headers(copy);
            }
        }
    }
}


SCENARIO("headers' copy-assignment operator copies", "[http_headers_t]") {
    GIVEN("a non-empty headers object") {
        httplib::http_headers_t headers {
            {"xxx", {"yyy"}},
            {"kkk", {"k"}}
        };

        REQUIRE(set_t({{"xxx", {"yyy"}}, {"kkk", {"k"}}}) == set_t(headers.begin(), headers.end()));

        WHEN("we copy-assign another non-empty object to it") {
            const httplib::http_headers_t another(non_empty_headers_init.begin(), non_empty_headers_init.end());

            headers = another;

            THEN("they both contain new headers") {
                validate_non_empty_headers(headers);
                validate_non_empty_headers(another);
            }
        }

        WHEN("we copy-assign an empty object to it") {
            const httplib::http_headers_t another;
            headers = another;

            THEN("they're both empty") {
                validate_empty_headers(headers);
                validate_empty_headers(another);
            }
        }
    }


    GIVEN("an empty headers object") {
        httplib::http_headers_t headers;

        WHEN("we copy-assign a non-empty object to it") {
            const httplib::http_headers_t another(non_empty_headers_init.begin(), non_empty_headers_init.end());

            headers = another;

            THEN("they both contain new headers") {
                validate_non_empty_headers(headers);
                validate_non_empty_headers(another);
            }
        }

        WHEN("we copy-assign an empty object to it") {
            const httplib::http_headers_t another;
            headers = another;

            THEN("they're both empty") {
                validate_empty_headers(headers);
                validate_empty_headers(another);
            }
        }
    }
}


SCENARIO("headers' move-assignment operator moves", "[http_headers_t]") {
    GIVEN("a non-empty headers object") {
        httplib::http_headers_t headers {
            {"xxx", {"yyy"}},
            {"kkk", {"k"}}
        };

        REQUIRE(set_t({{"xxx", {"yyy"}}, {"kkk", {"k"}}}) == set_t(headers.begin(), headers.end()));

        WHEN("we move-assign another non-empty object to it") {
            httplib::http_headers_t another(non_empty_headers_init.begin(), non_empty_headers_init.end());
            headers = std::move(another);

            THEN("it contains new headers") {
                validate_non_empty_headers(headers);
            }
        }

        WHEN("we move-assign an empty object to it") {
            httplib::http_headers_t another;
            headers = std::move(another);

            THEN("it becomes empty") {
                validate_empty_headers(headers);
            }
        }
    }


    GIVEN("an empty headers object") {
        httplib::http_headers_t headers;

        WHEN("we copy-assign a non-empty object to it") {
            httplib::http_headers_t another(non_empty_headers_init.begin(), non_empty_headers_init.end());
            headers = std::move(another);

            THEN("it contains new headers") {
                validate_non_empty_headers(headers);
            }
        }

        WHEN("we move-assign an empty object to it") {
            httplib::http_headers_t another;
            headers = std::move(another);

            THEN("it stays empty") {
                validate_empty_headers(headers);
            }
        }
    }
}


TEST_CASE("http_headers_t::size() denotes number of non-unique headers", "[http_headers_t]") {
    httplib::http_headers_t headers = {
        {"xxx", {"1", "2", "3"}},
        {"yyy", {"1", "2"}},
        {"XXX", {"1"}},
        {"zzz", {}}
    };

    REQUIRE(headers.size() == 6);

    headers.add_header_values("yyy", {"1", "2"});
    REQUIRE(headers.size() == 8);

    headers.add_header_values("kkk", {"1", "2"});
    REQUIRE(headers.size() == 10);

    headers.set_header("xxx", {"1"});
    REQUIRE(headers.size() == 7);

    headers.set_header("lll", {"1"});
    REQUIRE(headers.size() == 8);

    headers.remove_header("YYY");
    REQUIRE(headers.size() == 4);
}


TEST_CASE("http_headers_t::find()", "[http_headers_t]") {
    httplib::http_headers_t headers = {
        {"xxx", {"1", "2", "3"}},
        {"yyy", {"1", "2"}},
        {"XXX", {"1"}},
        {"zzz", {}}
    };

    REQUIRE(headers.find("abc") == headers.end());
    REQUIRE(headers.find("zzz") == headers.end());
    REQUIRE(headers.find("XXX") != headers.end());
    REQUIRE(headers.find("XXX")->first == "xxx");
    REQUIRE(headers.find("XXX")->first != "XXX"); // ¯\_(ツ)_/¯
    REQUIRE(headers.find("XXX")->second == httplib::http_headers_t::header_values_t({"1", "2", "3", "1"}));
    REQUIRE(headers.find("yyy") != headers.end());
    REQUIRE(headers.find("yyy")->first == "yyy");
    REQUIRE(headers.find("yyy")->second == httplib::http_headers_t::header_values_t({"1", "2"}));
}


TEST_CASE("http_headers_t::has()", "[http_headers_t]") {
    httplib::http_headers_t headers = {
        {"xxx", {"1", "2", "3"}},
        {"yyy", {"1", "2"}},
        {"XXX", {"1"}},
        {"zzz", {}}
    };

    REQUIRE(headers.has("xxx"));
    REQUIRE(headers.has("XXX"));
    REQUIRE(headers.has("yyy"));
    REQUIRE(headers.has("YyY"));
    REQUIRE(!headers.has(""));
    REQUIRE(!headers.has("abc"));
    REQUIRE(!headers.has("zzz"));
}


TEST_CASE("http_headers_t::get_header()", "[http_headers_t]") {
    httplib::http_headers_t headers = {
        {"xxx", {"1", "2", "3"}},
        {"yyy", {"1", "2"}},
        {"XXX", {"1"}},
        {"lll", {"1"}},
        {"kkk", {"2"}},
        {"zzz", {}}
    };

    // get_header() returns positive result only if there's a single value
    REQUIRE(!headers.get_header("xxx"));
    REQUIRE(!headers.get_header("yyy"));
    REQUIRE(!headers.get_header("XXX"));
    REQUIRE(!headers.get_header("zzz"));
    REQUIRE(headers.get_header("lll"));
    REQUIRE(*headers.get_header("lll") == "1");
    REQUIRE(headers.get_header("kkk"));
    REQUIRE(*headers.get_header("kkk") == "2");
}


TEST_CASE("http_headers_t::get_header_values()", "[http_headers_t]") {
    httplib::http_headers_t headers = {
        {"xxx", {"1", "2", "3"}},
        {"yyy", {"1", "2"}},
        {"XXX", {"1"}},
        {"lll", {"1"}},
        {"kkk", {"2"}},
        {"zzz", {}}
    };

    REQUIRE(headers.get_header_values("xxx"));
    REQUIRE(*headers.get_header_values("xxx") == httplib::http_headers_t::header_values_t({"1", "2", "3", "1"}));
    REQUIRE(headers.get_header_values("XXX"));
    REQUIRE(*headers.get_header_values("XXX") == httplib::http_headers_t::header_values_t({"1", "2", "3", "1"}));
    REQUIRE(headers.get_header_values("XxX"));
    REQUIRE(*headers.get_header_values("XxX") == httplib::http_headers_t::header_values_t({"1", "2", "3", "1"}));
    REQUIRE(headers.get_header_values("yyy"));
    REQUIRE(*headers.get_header_values("yyy") == httplib::http_headers_t::header_values_t({"1", "2"}));
    REQUIRE(headers.get_header_values("Yyy"));
    REQUIRE(*headers.get_header_values("Yyy") == httplib::http_headers_t::header_values_t({"1", "2"}));
    REQUIRE(headers.get_header_values("lll"));
    REQUIRE(*headers.get_header_values("lll") == httplib::http_headers_t::header_values_t({"1"}));
    REQUIRE(headers.get_header_values("LLL"));
    REQUIRE(*headers.get_header_values("LLL") == httplib::http_headers_t::header_values_t({"1"}));
    REQUIRE(headers.get_header_values("kkk"));
    REQUIRE(*headers.get_header_values("kkk") == httplib::http_headers_t::header_values_t({"2"}));
    REQUIRE(headers.get_header_values("kKk"));
    REQUIRE(*headers.get_header_values("kKk") == httplib::http_headers_t::header_values_t({"2"}));
    REQUIRE(!headers.get_header_values("zzz"));
    REQUIRE(!headers.get_header_values("abc"));
    REQUIRE(!headers.get_header_values(""));
}


TEST_CASE("http_headers_t::set_header() replaces values", "[http_headers_t]") {
    httplib::http_headers_t headers = {
        {"xxx", {"1", "2", "3"}},
        {"yyy", {"1", "2"}},
        {"XXX", {"1"}},
        {"lll", {"1"}},
        {"kkk", {"2"}},
        {"zzz", {}}
    };

    REQUIRE(headers.size() == 8);
    REQUIRE(headers.get_header_values("xxx"));
    REQUIRE(*headers.get_header_values("xxx") == httplib::http_headers_t::header_values_t({"1", "2", "3", "1"}));

    headers.set_header("xXx", {"kk", "k"});

    REQUIRE(headers.size() == 6);
    REQUIRE(headers.get_header_values("xxx"));
    REQUIRE(*headers.get_header_values("xxx") == httplib::http_headers_t::header_values_t({"kk", "k"}));
}


TEST_CASE("http_headers_t::set_header() creates a header", "[http_headers_t]") {
    httplib::http_headers_t headers = {
        {"xxx", {"1", "2", "3"}},
        {"yyy", {"1", "2"}},
        {"XXX", {"1"}},
        {"lll", {"1"}},
        {"kkk", {"2"}},
        {"zzz", {}}
    };

    REQUIRE(headers.size() == 8);
    REQUIRE(!headers.has("ttt"));

    headers.set_header("ttt", {"kk", "k"});

    REQUIRE(headers.size() == 10);
    REQUIRE(headers.get_header_values("tTt"));
    REQUIRE(*headers.get_header_values("tTt") == httplib::http_headers_t::header_values_t({"kk", "k"}));
}


TEST_CASE("http_headers_t::set_header() with zero values removes a header", "[http_headers_t]") {
    httplib::http_headers_t headers = {
        {"xxx", {"1", "2", "3"}},
        {"yyy", {"1", "2"}},
        {"XXX", {"1"}},
        {"lll", {"1"}},
        {"kkk", {"2"}},
        {"zzz", {}}
    };

    REQUIRE(headers.size() == 8);
    REQUIRE(headers.get_header_values("xxx"));
    REQUIRE(*headers.get_header_values("xxx") == httplib::http_headers_t::header_values_t({"1", "2", "3", "1"}));

    headers.set_header("xXx", {});

    REQUIRE(headers.size() == 4);
    REQUIRE(!headers.has("xxx"));
    REQUIRE(!headers.has("xXx"));
    REQUIRE(!headers.has("XXX"));
    REQUIRE(headers.find("xxx") == headers.end());
    REQUIRE(headers.find("xXx") == headers.end());
    REQUIRE(!headers.get_header("xxx"));
    REQUIRE(!headers.get_header("xXx"));
    REQUIRE(!headers.get_header_values("xxx"));
    REQUIRE(!headers.get_header_values("xXx"));
}


TEST_CASE("http_headers_t::add_header_values() adds values", "[http_headers_t]") {
    httplib::http_headers_t headers = {
        {"xxx", {"1", "2", "3"}},
        {"yyy", {"1", "2"}},
        {"XXX", {"1"}},
        {"lll", {"1"}},
        {"kkk", {"2"}},
        {"zzz", {}}
    };

    REQUIRE(headers.size() == 8);
    REQUIRE(headers.get_header_values("yyy"));
    REQUIRE(*headers.get_header_values("yyy") == httplib::http_headers_t::header_values_t({"1", "2"}));

    headers.add_header_values("yyy", {"3", "4"});

    REQUIRE(headers.size() == 10);
    REQUIRE(headers.get_header_values("yyy"));
    REQUIRE(*headers.get_header_values("yyy") == httplib::http_headers_t::header_values_t({"1", "2", "3", "4"}));

    headers.add_header_values("YyY", {"5"});

    REQUIRE(headers.size() == 11);
    REQUIRE(headers.get_header_values("yyy"));
    REQUIRE(*headers.get_header_values("yyy") == httplib::http_headers_t::header_values_t({"1", "2", "3", "4", "5"}));

    headers.add_header_values("YyY", {});

    REQUIRE(headers.size() == 11);
    REQUIRE(headers.get_header_values("yyy"));
    REQUIRE(*headers.get_header_values("yyy") == httplib::http_headers_t::header_values_t({"1", "2", "3", "4", "5"}));
}


TEST_CASE("http_headers_t::add_header_values() creates a header", "[http_headers_t]") {
    httplib::http_headers_t headers = {
        {"xxx", {"1", "2", "3"}},
        {"yyy", {"1", "2"}},
        {"XXX", {"1"}},
        {"lll", {"1"}},
        {"kkk", {"2"}},
        {"zzz", {}}
    };

    REQUIRE(headers.size() == 8);
    REQUIRE(!headers.has("ttt"));

    headers.add_header_values("ttt", {"3", "4"});

    REQUIRE(headers.size() == 10);
    REQUIRE(headers.get_header_values("ttt"));
    REQUIRE(*headers.get_header_values("ttt") == httplib::http_headers_t::header_values_t({"3", "4"}));

    headers.add_header_values("ttt", {"5"});

    REQUIRE(headers.size() == 11);
    REQUIRE(headers.get_header_values("ttt"));
    REQUIRE(*headers.get_header_values("ttt") == httplib::http_headers_t::header_values_t({"3", "4", "5"}));
}


TEST_CASE("http_headers_t::add_header_values() with zero values doesn't create a header", "[http_headers_t]") {
    httplib::http_headers_t headers = {
        {"xxx", {"1", "2", "3"}},
        {"yyy", {"1", "2"}},
        {"XXX", {"1"}},
        {"lll", {"1"}},
        {"kkk", {"2"}},
        {"zzz", {}}
    };

    REQUIRE(headers.size() == 8);
    REQUIRE(!headers.has("ttt"));

    headers.add_header_values("ttt", {});

    REQUIRE(headers.size() == 8);
    REQUIRE(headers.find("ttt") == headers.end());
    REQUIRE(!headers.has("ttt"));
    REQUIRE(!headers.get_header("ttt"));
    REQUIRE(!headers.get_header_values("ttt"));
}


TEST_CASE("http_headers_t::remove_header() removes", "[http_headers_t]") {
    httplib::http_headers_t headers = {
        {"xxx", {"1", "2", "3"}},
        {"yyy", {"1", "2"}},
        {"XXX", {"1"}},
        {"lll", {"1"}},
        {"kkk", {"2"}},
        {"zzz", {}}
    };

    REQUIRE(headers.size() == 8);
    REQUIRE(headers.has("xxx"));
    REQUIRE(headers.has("yyy"));
    REQUIRE(headers.has("lll"));
    REQUIRE(headers.has("kkk"));

    headers.remove_header("xxx");

    REQUIRE(!headers.empty());
    REQUIRE(headers.size() == 4);
    REQUIRE(!headers.has("xxx"));
    REQUIRE(!headers.has("XXX"));
    REQUIRE(headers.find("xxx") == headers.end());
    REQUIRE(!headers.get_header("xxx"));
    REQUIRE(!headers.get_header_values("xxx"));

    headers.remove_header("YyY");

    REQUIRE(!headers.empty());
    REQUIRE(headers.size() == 2);
    REQUIRE(!headers.has("yyy"));
    REQUIRE(!headers.has("YyY"));
    REQUIRE(headers.find("yyy") == headers.end());
    REQUIRE(!headers.get_header("yyy"));
    REQUIRE(!headers.get_header_values("yyy"));

    headers.remove_header("LLL");
    headers.remove_header("kkk");

    REQUIRE(headers.empty());
    REQUIRE(headers.size() == 0);
}


TEST_CASE("headers' output operator", "[http_headers_t]") {
    httplib::http_headers_t headers = {
        {"xxx", {"1", "2", "3"}},
        {"yyy", {"1", "2"}},
        {"XXX", {"1"}},
        {"lll", {"1"}},
        {"kkk", {"2"}},
        {"zzz", {}}
    };

    std::string expected =
        "kkk: 2\r\n"
        "lll: 1\r\n"
        "xxx: 1\r\n"
        "xxx: 2\r\n"
        "xxx: 3\r\n"
        "xxx: 1\r\n"
        "yyy: 1\r\n"
        "yyy: 2\r\n";

    REQUIRE(headers_to_string(headers) == expected);
}


TEST_CASE("headers' output operator accepts empty headers object", "[http_headers_t]") {
    httplib::http_headers_t headers;

    std::string expected = "";

    REQUIRE(headers_to_string(headers) == expected);
}


TEST_CASE("headers' output operator accepts empty headers", "[http_headers_t]") {
    httplib::http_headers_t headers = {
        {"xxx", {"1", "2", "3"}},
        {"yyy", {"1", "2"}},
        {"XXX", {"1"}},
        {"lll", {""}},
        {"kkk", {"2"}},
        {"zzz", {}}
    };

    std::string expected =
        "kkk: 2\r\n"
        "lll: \r\n"
        "xxx: 1\r\n"
        "xxx: 2\r\n"
        "xxx: 3\r\n"
        "xxx: 1\r\n"
        "yyy: 1\r\n"
        "yyy: 2\r\n";

    REQUIRE(headers_to_string(headers) == expected);
}


TEST_CASE("headers' output operator puts Home first", "[http_headers_t]") {
    httplib::http_headers_t headers = {
        {"aaa", {"1", "2", "3"}},
        {"yyy", {"1", "2"}},
        {"AAA", {"1"}},
        {"hOme", {"1"}},
        {"lll", {""}},
        {"Home", {"abc", "de"}},
        {"kkk", {"2"}},
        {"zzz", {}}
    };

    std::string expected =
        "hOme: 1\r\n"
        "hOme: abc\r\n"
        "hOme: de\r\n"
        "aaa: 1\r\n"
        "aaa: 2\r\n"
        "aaa: 3\r\n"
        "aaa: 1\r\n"
        "kkk: 2\r\n"
        "lll: \r\n"
        "yyy: 1\r\n"
        "yyy: 2\r\n";

    REQUIRE(headers_to_string(headers) == expected);
}
