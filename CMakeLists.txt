cmake_minimum_required(VERSION 3.5)

project(flb-archiver LANGUAGES C)

add_executable(flb-archiver src/main.c)

find_package(CURL REQUIRED)
target_link_libraries(flb-archiver PUBLIC CURL::libcurl)

install(TARGETS flb-archiver
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
