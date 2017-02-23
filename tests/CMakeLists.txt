ADD_CUSTOM_TARGET(check
    ctest -V
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)


ADD_EXECUTABLE(unittests
    common.cpp
    result.cpp
)

TARGET_INCLUDE_DIRECTORIES(unittests SYSTEM PRIVATE
    ${PROJECT_SOURCE_DIR}/contrib/Catch-1.7.2/single_include
)

TARGET_LINK_LIBRARIES(unittests ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} httplib)
TARGET_COMPILE_OPTIONS(unittests PRIVATE -std=c++14 -Wall -Wextra -Werror -pedantic -pedantic-errors)

ADD_TEST(NAME unittests COMMAND unittests)
ADD_DEPENDENCIES(check unittests)
