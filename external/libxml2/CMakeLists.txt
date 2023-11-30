include(FetchContent)

# https://github.com/GNOME/libxml2/tags
set(libxml2_version 2.12.1)
set(libxml2_sha256 9815c91fcfab336b1e2a6ba506cf38c5baa6817c9ed4f30211cbc9011406d9fb)

set(BUILD_SHARED_LIBS OFF)
set(LIBXML2_WITH_DEBUG OFF)
set(LIBXML2_WITH_PROGRAMS OFF)
set(LIBXML2_WITH_PYTHON OFF)
set(LIBXML2_WITH_TESTS OFF)

FetchContent_Declare(libxml2
  URL https://github.com/GNOME/libxml2/archive/refs/tags/v${libxml2_version}.tar.gz
  URL_HASH SHA256=${libxml2_sha256}
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}
)

FetchContent_MakeAvailable(libxml2)
