set(target_name flb-archiver)

add_executable(${target_name})

include(CompileOptions)
set_compile_options(${target_name})

target_sources(${target_name} PRIVATE main.c)

target_include_directories(${target_name} PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)

find_package(LibXml2 REQUIRED)
find_package(CURL REQUIRED)

target_link_libraries(${target_name} PRIVATE
  filesystem
  download
)

set_target_properties(${target_name} PROPERTIES
  LINK_FLAGS_RELEASE -s
)
