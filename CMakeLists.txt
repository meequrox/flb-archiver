cmake_minimum_required(VERSION 3.5)

project(flb-archiver LANGUAGES C)
set(CMAKE_C_STANDARD 11)

add_executable(flb-archiver src/main.c)
target_sources(flb-archiver PRIVATE
    src/linkpool.h

    src/linkpool.c
)

find_package(LibXml2 REQUIRED)
target_link_libraries(flb-archiver PUBLIC LibXml2::LibXml2)

find_package(CURL REQUIRED)
target_link_libraries(flb-archiver PUBLIC CURL::libcurl)

install(TARGETS flb-archiver
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
