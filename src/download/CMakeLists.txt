set(target_name download)

set(LIBRARY_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/${target_name})

add_library(${target_name} STATIC
  ${LIBRARY_INCLUDE_DIR}/comments.h
  ${LIBRARY_INCLUDE_DIR}/thread.h
  comments.c
  thread.c
)

target_include_directories(${target_name} PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  ${CMAKE_BINARY_DIR}/_deps/libxml2-src/include
)

if(NOT WIN32)
  find_package(CURL REQUIRED)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(${target_name} PUBLIC
  memory
  filesystem
  data_structures
  LibXml2::LibXml2
  CURL::libcurl
  Threads::Threads
)

include(CompileOptions)
set_compile_options(${target_name})
