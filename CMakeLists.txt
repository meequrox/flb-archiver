cmake_minimum_required(VERSION 3.5)

file(READ "include/version.h" FLB_VERSION_HEADER)

string(REGEX MATCH "#define FLB_VERSION_MAJOR \"([0-9]*)\"" _ ${FLB_VERSION_HEADER})
set(FLB_VERSION_MAJOR ${CMAKE_MATCH_1})

string(REGEX MATCH "#define FLB_VERSION_MINOR \"([0-9]*)\"" _ ${FLB_VERSION_HEADER})
set(FLB_VERSION_MINOR ${CMAKE_MATCH_1})

string(REGEX MATCH "#define FLB_VERSION_PATCH \"([0-9]*)\"" _ ${FLB_VERSION_HEADER})
set(FLB_VERSION_PATCH ${CMAKE_MATCH_1})

project(flb-archiver VERSION ${FLB_VERSION_MAJOR}.${FLB_VERSION_MINOR}.${FLB_VERSION_PATCH})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if (NOT CLANG_TIDY_EXE)
    message(WARNING "clang-tidy not found")
else ()
    execute_process(COMMAND
        ${CLANG_TIDY_EXE} --version
        OUTPUT_VARIABLE CLANG_TIDY_VERSION
    )
    message("clang-tidy found:\n" ${CLANG_TIDY_VERSION})
endif ()

add_subdirectory(src)
