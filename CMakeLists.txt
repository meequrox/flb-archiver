cmake_minimum_required(VERSION 3.5)

project(flb-archiver LANGUAGES C)

add_executable(flb-archiver src/main.c)

install(TARGETS flb-archiver
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
