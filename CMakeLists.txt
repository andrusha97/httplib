CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(HTTPLIB)


IF(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    SET(STANDALONE_BUILD ON)
ELSE()
    SET(STANDALONE_BUILD OFF)
ENDIF()


IF(STANDALONE_BUILD)
    OPTION(ENABLE_TESTS "Enable tests" ON)
    OPTION(BUILD_EXAMPLES "Build examples" OFF)

    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    SET(Boost_USE_MULTITHREADED ON)

    FIND_PACKAGE(Threads REQUIRED)
    FIND_PACKAGE(Boost 1.61.0 REQUIRED COMPONENTS system)
ENDIF()


ADD_LIBRARY(httplib
    ${PROJECT_SOURCE_DIR}/contrib/http-parser-2.7.1/http_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/error.cpp
    ${PROJECT_SOURCE_DIR}/src/http/headers.cpp
    ${PROJECT_SOURCE_DIR}/src/http/message_properties.cpp
    ${PROJECT_SOURCE_DIR}/src/http/request.cpp
    ${PROJECT_SOURCE_DIR}/src/http/response.cpp
    ${PROJECT_SOURCE_DIR}/src/http/status_code.cpp
    ${PROJECT_SOURCE_DIR}/src/http/url.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/chunked_body_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/detail/utility.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/extension_list_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/request_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/response_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/token_list_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/response_builder.cpp
)

TARGET_INCLUDE_DIRECTORIES(httplib BEFORE PUBLIC ${PROJECT_SOURCE_DIR}/include)

TARGET_INCLUDE_DIRECTORIES(httplib SYSTEM
    PRIVATE ${PROJECT_SOURCE_DIR}/contrib/http-parser-2.7.1/include
    PUBLIC ${Boost_INCLUDE_DIRS}
)

TARGET_COMPILE_OPTIONS(httplib PRIVATE -std=c++14 -pedantic -pedantic-errors -Wall -Wextra -Werror)


IF(STANDALONE_BUILD AND ENABLE_TESTS)
    ENABLE_TESTING()

    ADD_SUBDIRECTORY(tests)

    ADD_EXECUTABLE(http-parser-test
        ${PROJECT_SOURCE_DIR}/contrib/http-parser-2.7.1/http_parser.cpp
        ${PROJECT_SOURCE_DIR}/contrib/http-parser-2.7.1/test.cpp
    )

    TARGET_INCLUDE_DIRECTORIES(http-parser-test BEFORE PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/contrib/http-parser-2.7.1/include
    )

    TARGET_COMPILE_OPTIONS(http-parser-test PRIVATE -std=c++14 -Wall -Werror)

    ADD_TEST(NAME http-parser-test COMMAND http-parser-test)
    ADD_DEPENDENCIES(check http-parser-test)
ENDIF()


IF(STANDALONE_BUILD AND BUILD_EXAMPLES)
    ADD_SUBDIRECTORY(examples)
ENDIF()
