set(target_name logger)

set(LIBRARY_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/${target_name})

add_library(${target_name} STATIC
  ${LIBRARY_INCLUDE_DIR}/colors.h
  ${LIBRARY_INCLUDE_DIR}/logger.h
  logger.c
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(${target_name} PUBLIC
  Threads::Threads
)

target_include_directories(${target_name} PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)

include(CompileOptions)
set_compile_options(${target_name})
