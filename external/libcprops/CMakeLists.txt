include(FetchContent)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# https://sourceforge.net/projects/cprops/files/cprops/
set(libcprops_version 0.1.12)
set(libcprops_sha256 16a0fa8267008a3e6d253d3809b53bac3f0b5f33968351446348b624ac6442f0)

FetchContent_Declare(libcprops
    URL https://master.dl.sourceforge.net/project/cprops/cprops/cprops-${libcprops_version}/libcprops-${libcprops_version}.tar.gz
    URL_HASH SHA256=${libcprops_sha256}
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
)

FetchContent_MakeAvailable(libcprops)

execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${libcprops_SOURCE_DIR} ./configure
    --prefix=${CMAKE_CURRENT_BINARY_DIR}/build
    --mandir=${CMAKE_CURRENT_BINARY_DIR}/build/man
    --enable-shared=no
    --enable-ssl=no
    --enable-cp-dbms=no
    --enable-cpsvc=no
    --enable-cpsp=no
    --with-pic=yes
)

execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${libcprops_SOURCE_DIR} make)
execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${libcprops_SOURCE_DIR} make install)
