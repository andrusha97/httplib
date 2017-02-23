#include <catch.hpp>

#include <httplib/result.hpp>

#include <memory>
#include <string>
#include <utility>


TEST_CASE("default-initialized result contains an error", "[result]") {
    httplib::result<int, std::string> result;

    REQUIRE(result.is_error());
    REQUIRE(!static_cast<bool>(result));
    REQUIRE(result.error() == "");
}


TEST_CASE("result with a value", "[result]") {
    httplib::result<std::unique_ptr<int>, std::string> result = std::make_unique<int>(5);

    REQUIRE(!result.is_error());
    REQUIRE(static_cast<bool>(result));
    REQUIRE(result.value());
    REQUIRE(*result.value() == 5);
    REQUIRE(*result);
    REQUIRE(**result == 5);
    REQUIRE(result->get());
}


TEST_CASE("const result with a value", "[result]") {
    const httplib::result<std::unique_ptr<int>, std::string> result = std::make_unique<int>(5);

    REQUIRE(!result.is_error());
    REQUIRE(static_cast<bool>(result));
    REQUIRE(result.value());
    REQUIRE(*result.value() == 5);
    REQUIRE(*result);
    REQUIRE(**result == 5);
    REQUIRE(result->get());
}


TEST_CASE("result with an error", "[result]") {
    using result_t = httplib::result<std::unique_ptr<int>, std::string>;
    result_t result = httplib::make_error_result<result_t>("abc");

    REQUIRE(result.is_error());
    REQUIRE(!static_cast<bool>(result));
    REQUIRE(result.error() == "abc");
}


TEST_CASE("const result with an error", "[result]") {
    using result_t = httplib::result<std::unique_ptr<int>, std::string>;
    const result_t result = httplib::make_error_result<result_t>("abc");

    REQUIRE(result.is_error());
    REQUIRE(!static_cast<bool>(result));
    REQUIRE(result.error() == "abc");
}


SCENARIO("result's copy constructor copies", "[result]") {
    GIVEN("a result with an error") {
        using result_t = httplib::result<int, std::string>;
        const result_t result = httplib::make_error_result<result_t>("abc");
        REQUIRE(!static_cast<bool>(result));
        REQUIRE(result.error() == "abc");

        WHEN("we copy-construct another result from it") {
            httplib::result<int, std::string> another_result = result;

            THEN("they both contain the error") {
                REQUIRE(result.is_error());
                REQUIRE(!static_cast<bool>(result));
                REQUIRE(result.error() == "abc");

                REQUIRE(another_result.is_error());
                REQUIRE(!static_cast<bool>(another_result));
                REQUIRE(another_result.error() == "abc");
            }
        }
    }

    GIVEN("a result with a value") {
        const httplib::result<int, std::string> result = 5;
        REQUIRE(static_cast<bool>(result));
        REQUIRE(*result == 5);

        WHEN("we copy-construct another result from it") {
            httplib::result<int, std::string> another_result = result;

            THEN("they both contain the value") {
                REQUIRE(!result.is_error());
                REQUIRE(static_cast<bool>(result));
                REQUIRE(*result == 5);
                REQUIRE(result.value() == 5);

                REQUIRE(!another_result.is_error());
                REQUIRE(static_cast<bool>(another_result));
                REQUIRE(*another_result == 5);
                REQUIRE(another_result.value() == 5);
            }
        }
    }
}


SCENARIO("result's move constructor moves", "[result]") {
    GIVEN("a result with an error") {
        using result_t = httplib::result<std::unique_ptr<int>, std::string>;
        result_t result = httplib::make_error_result<result_t>("abc");
        REQUIRE(!static_cast<bool>(result));
        REQUIRE(result.error() == "abc");

        WHEN("we move-construct another result from it") {
            httplib::result<std::unique_ptr<int>, std::string> another_result = std::move(result);

            THEN("new result contains the error") {
                REQUIRE(another_result.is_error());
                REQUIRE(!static_cast<bool>(another_result));
                REQUIRE(another_result.error() == "abc");
            }
        }
    }

    GIVEN("a result with a value") {
        httplib::result<std::unique_ptr<int>, std::string> result = std::make_unique<int>(5);

        REQUIRE(static_cast<bool>(result));
        REQUIRE(*result);
        REQUIRE(**result == 5);

        WHEN("we move-construct another result from it") {
            httplib::result<std::unique_ptr<int>, std::string> another_result = std::move(result);

            THEN("new result contains the value") {
                REQUIRE(!another_result.is_error());
                REQUIRE(static_cast<bool>(another_result));
                REQUIRE(another_result.value());
                REQUIRE(*another_result.value() == 5);
                REQUIRE(*another_result);
                REQUIRE(**another_result == 5);
                REQUIRE(another_result->get());
            }
        }
    }
}


SCENARIO("result's copy assignment copies", "[result]") {
    GIVEN("a result with an error") {
        using result_t = httplib::result<int, std::string>;
        result_t result = httplib::make_error_result<result_t>("abc");
        REQUIRE(!static_cast<bool>(result));
        REQUIRE(result.error() == "abc");

        WHEN("we copy-assign an error result to it") {
            const result_t error_result = httplib::make_error_result<result_t>("defg");
            REQUIRE(!static_cast<bool>(error_result));
            REQUIRE(error_result.error() == "defg");

            result = error_result;

            THEN("it contains the new error") {
                REQUIRE(result.is_error());
                REQUIRE(!static_cast<bool>(result));
                REQUIRE(result.error() == "defg");
            }
        }

        WHEN("we copy-assign a value result to it") {
            const httplib::result<int, std::string> value_result = 1337;
            REQUIRE(static_cast<bool>(value_result));
            REQUIRE(value_result.value() == 1337);

            result = value_result;

            THEN("it contains the value") {
                REQUIRE(!result.is_error());
                REQUIRE(static_cast<bool>(result));
                REQUIRE(*result == 1337);
                REQUIRE(result.value() == 1337);
            }
        }
    }

    GIVEN("a result with a value") {
        httplib::result<int, std::string> result = 5;
        REQUIRE(static_cast<bool>(result));
        REQUIRE(*result == 5);

        WHEN("we copy-assign an error result to it") {
            using result_t = httplib::result<int, std::string>;
            const result_t error_result = httplib::make_error_result<result_t>("defg");
            REQUIRE(!static_cast<bool>(error_result));
            REQUIRE(error_result.error() == "defg");

            result = error_result;

            THEN("it contains the error") {
                REQUIRE(result.is_error());
                REQUIRE(!static_cast<bool>(result));
                REQUIRE(result.error() == "defg");
            }
        }

        WHEN("we copy-assign a value result to it") {
            const httplib::result<int, std::string> value_result = 1337;
            REQUIRE(static_cast<bool>(value_result));
            REQUIRE(value_result.value() == 1337);

            result = value_result;

            THEN("it contains the new value") {
                REQUIRE(!result.is_error());
                REQUIRE(static_cast<bool>(result));
                REQUIRE(*result == 1337);
                REQUIRE(result.value() == 1337);
            }
        }
    }
}


SCENARIO("result's move assignment moves", "[result]") {
    GIVEN("a result with an error") {
        using result_t = httplib::result<std::unique_ptr<int>, std::string>;
        result_t result = httplib::make_error_result<result_t>("abc");
        REQUIRE(!static_cast<bool>(result));
        REQUIRE(result.error() == "abc");

        WHEN("we copy-assign an error result to it") {
            result_t error_result = httplib::make_error_result<result_t>("defg");
            REQUIRE(!static_cast<bool>(error_result));
            REQUIRE(error_result.error() == "defg");

            result = std::move(error_result);

            THEN("it contains the new error") {
                REQUIRE(result.is_error());
                REQUIRE(!static_cast<bool>(result));
                REQUIRE(result.error() == "defg");
            }
        }

        WHEN("we copy-assign a value result to it") {
            httplib::result<std::unique_ptr<int>, std::string> value_result = std::make_unique<int>(1337);
            REQUIRE(*value_result);
            REQUIRE(**value_result == 1337);

            result = std::move(value_result);

            THEN("it contains the value") {
                REQUIRE(!result.is_error());
                REQUIRE(static_cast<bool>(result));
                REQUIRE(result.value());
                REQUIRE(*result.value() == 1337);
                REQUIRE(*result);
                REQUIRE(**result == 1337);
                REQUIRE(result->get());
            }
        }
    }

    GIVEN("a result with a value") {
        httplib::result<std::unique_ptr<int>, std::string> result = std::make_unique<int>(5);
        REQUIRE(static_cast<bool>(result));
        REQUIRE(*result);
        REQUIRE(**result == 5);

        WHEN("we copy-assign an error result to it") {
            using result_t = httplib::result<std::unique_ptr<int>, std::string>;
            result_t error_result = httplib::make_error_result<result_t>("defg");
            REQUIRE(!static_cast<bool>(error_result));
            REQUIRE(error_result.error() == "defg");

            result = std::move(error_result);

            THEN("it contains the error") {
                REQUIRE(result.is_error());
                REQUIRE(!static_cast<bool>(result));
                REQUIRE(result.error() == "defg");
            }
        }

        WHEN("we copy-assign a value result to it") {
            httplib::result<std::unique_ptr<int>, std::string> value_result = std::make_unique<int>(1337);
            REQUIRE(*value_result);
            REQUIRE(**value_result == 1337);

            result = std::move(value_result);

            THEN("it contains the new value") {
                REQUIRE(!result.is_error());
                REQUIRE(static_cast<bool>(result));
                REQUIRE(result.value());
                REQUIRE(*result.value() == 1337);
                REQUIRE(*result);
                REQUIRE(**result == 1337);
                REQUIRE(result->get());
            }
        }
    }
}


TEST_CASE("make_error_result", "[result]") {
    SECTION("same types") {
        using result_t = httplib::result<std::string, std::string>;
        result_t result = httplib::make_error_result<result_t>("abc");

        REQUIRE(result.is_error());
        REQUIRE(!static_cast<bool>(result));
        REQUIRE(result.error() == "abc");
    }

    SECTION("different types") {
        using result_t = httplib::result<std::unique_ptr<int>, std::string>;
        result_t result = httplib::make_error_result<result_t>("abc");

        REQUIRE(result.is_error());
        REQUIRE(!static_cast<bool>(result));
        REQUIRE(result.error() == "abc");
    }
}


TEST_CASE("make_result", "[result]") {
    SECTION("same types") {
        using result_t = httplib::result<std::string, std::string>;
        result_t result = httplib::make_result<result_t>("abc");

        REQUIRE(!result.is_error());
        REQUIRE(static_cast<bool>(result));
        REQUIRE(result.value() == "abc");
        REQUIRE(*result == "abc");
        REQUIRE(result->size() == 3);
    }

    SECTION("different types") {
        using result_t = httplib::result<std::unique_ptr<int>, std::string>;
        result_t result = httplib::make_result<result_t>(new int(5));

        REQUIRE(!result.is_error());
        REQUIRE(static_cast<bool>(result));
        REQUIRE(result.value());
        REQUIRE(*result.value() == 5);
        REQUIRE(*result);
        REQUIRE(**result == 5);
        REQUIRE(result->get());
    }
}


TEST_CASE("result's value type and error type may be the same", "[result]") {
    SECTION("result with an error") {
        using result_t = httplib::result<std::string, std::string>;
        result_t result = httplib::make_error_result<result_t>("abc");

        REQUIRE(result.is_error());
        REQUIRE(!static_cast<bool>(result));
        REQUIRE(result.error() == "abc");
    }

    SECTION("result with a value") {
        using result_t = httplib::result<std::string, std::string>;
        result_t result = httplib::make_result<result_t>("abc");

        REQUIRE(!result.is_error());
        REQUIRE(static_cast<bool>(result));
        REQUIRE(result.value() == "abc");
        REQUIRE(*result == "abc");
        REQUIRE(std::string(result->data()) == "abc");
    }

    SECTION("implicitly constructed result with a value") {
        using result_t = httplib::result<std::string, std::string>;
        result_t result = std::string("abc");

        REQUIRE(!result.is_error());
        REQUIRE(static_cast<bool>(result));
        REQUIRE(result.value() == "abc");
        REQUIRE(*result == "abc");
        REQUIRE(std::string(result->data()) == "abc");
    }
}


TEST_CASE("result's equality operator", "[result]") {
    SECTION("same errors") {
        using result_t = httplib::result<int, std::string>;
        const result_t result1 = httplib::make_error_result<result_t>("abc");
        const result_t result2 = httplib::make_error_result<result_t>("abc");
        REQUIRE(result1 == result2);
    }

    SECTION("different errors") {
        using result_t = httplib::result<int, std::string>;
        const result_t result1 = httplib::make_error_result<result_t>("abc");
        const result_t result2 = httplib::make_error_result<result_t>("def");
        REQUIRE(!(result1 == result2));
    }

    SECTION("same values") {
        const httplib::result<int, std::string> result1 = 5;
        const httplib::result<int, std::string> result2 = 5;
        REQUIRE(result1 == result2);
    }

    SECTION("different values") {
        const httplib::result<int, std::string> result1 = 5;
        const httplib::result<int, std::string> result2 = 15;
        REQUIRE(!(result1 == result2));
    }

    SECTION("implicit conversions to values") {
        const httplib::result<int, std::string> result = 5;
        REQUIRE(result == 5);
        REQUIRE(5 == result);
        REQUIRE(!(result == 15));
        REQUIRE(!(15 == result));
    }

    SECTION("value and error") {
        using result_t = httplib::result<int, std::string>;
        const result_t result1 = 5;
        const result_t result2 = httplib::make_error_result<result_t>("abc");
        REQUIRE(!(result1 == result2));
        REQUIRE(!(result2 == result1));
        REQUIRE(!(result1 == httplib::make_error_result<result_t>("abc")));
        REQUIRE(!(httplib::make_error_result<result_t>("abc") == result1));
        REQUIRE(!(result2 == 5));
        REQUIRE(!(5 == result2));
    }

    SECTION("value and error of the same type") {
        using result_t = httplib::result<std::string, std::string>;
        const result_t result1 = httplib::make_result<result_t>("abc");
        const result_t result2 = httplib::make_error_result<result_t>("abc");
        REQUIRE(!(result1 == result2));
        REQUIRE(!(result2 == result1));
    }
}


TEST_CASE("result's inequality operator", "[result]") {
    SECTION("same errors") {
        using result_t = httplib::result<int, std::string>;
        const result_t result1 = httplib::make_error_result<result_t>("abc");
        const result_t result2 = httplib::make_error_result<result_t>("abc");
        REQUIRE(!(result1 != result2));
    }

    SECTION("different errors") {
        using result_t = httplib::result<int, std::string>;
        const result_t result1 = httplib::make_error_result<result_t>("abc");
        const result_t result2 = httplib::make_error_result<result_t>("def");
        REQUIRE(result1 != result2);
    }

    SECTION("same values") {
        const httplib::result<int, std::string> result1 = 5;
        const httplib::result<int, std::string> result2 = 5;
        REQUIRE(!(result1 != result2));
    }

    SECTION("different values") {
        const httplib::result<int, std::string> result1 = 5;
        const httplib::result<int, std::string> result2 = 15;
        REQUIRE(result1 != result2);
    }

    SECTION("implicit conversions to values") {
        const httplib::result<int, std::string> result = 5;
        REQUIRE(!(result != 5));
        REQUIRE(!(5 != result));
        REQUIRE(result != 15);
        REQUIRE(15 != result);
    }

    SECTION("value and error") {
        using result_t = httplib::result<int, std::string>;
        const result_t result1 = 5;
        const result_t result2 = httplib::make_error_result<result_t>("abc");
        REQUIRE(result1 != result2);
        REQUIRE(result2 != result1);
        REQUIRE(result1 != httplib::make_error_result<result_t>("abc"));
        REQUIRE(httplib::make_error_result<result_t>("abc") != result1);
        REQUIRE(result2 != 5);
        REQUIRE(5 != result2);
    }

    SECTION("value and error of the same type") {
        using result_t = httplib::result<std::string, std::string>;
        const result_t result1 = httplib::make_result<result_t>("abc");
        const result_t result2 = httplib::make_error_result<result_t>("abc");
        REQUIRE(result1 != result2);
        REQUIRE(result2 != result1);
    }
}
