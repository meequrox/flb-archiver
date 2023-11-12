set(target_name flb-archiver)

add_executable(${target_name})

include(CompileOptions)
set_compile_options(${target_name})

target_sources(${target_name} PRIVATE main.c)

find_package(LibXml2 REQUIRED)
find_package(CURL REQUIRED)

target_link_libraries(${target_name} PRIVATE
    linkpool
    memory
    LibXml2::LibXml2
    ${CURL_LIBRARIES}
)

set_target_properties(${target_name} PROPERTIES
    LINK_FLAGS_RELEASE -s
)
