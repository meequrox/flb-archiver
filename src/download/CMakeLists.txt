set(target_name download)

set(LIBRARY_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/${target_name})

add_library(${target_name} STATIC
    ${LIBRARY_INCLUDE_DIR}/comments.h
    ${LIBRARY_INCLUDE_DIR}/thread.h
    comments.c
    thread.c
)

set(LIBCPROPS_INCLUDE_DIR ${CMAKE_BINARY_DIR}/external/libcprops/build/include)
set(LIBCPROPS_LIBRARY_DIR ${CMAKE_BINARY_DIR}/external/libcprops/build/lib)

target_include_directories(${target_name} PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${LIBCPROPS_INCLUDE_DIR}
)

find_package(LibXml2 REQUIRED)
find_package(CURL REQUIRED)

target_link_directories(${target_name} PUBLIC ${LIBCPROPS_LIBRARY_DIR})

target_link_libraries(${target_name} PUBLIC
    memory
    filesystem
    cprops
    LibXml2::LibXml2
    ${CURL_LIBRARIES}
)

include(CompileOptions)
set_compile_options(${target_name})
