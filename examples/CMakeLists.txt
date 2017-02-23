ADD_EXECUTABLE(server
    server.cpp
)

TARGET_LINK_LIBRARIES(server ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} httplib)
TARGET_COMPILE_OPTIONS(server PRIVATE -std=c++14 -pedantic -pedantic-errors -Wall -Wextra -Werror)
