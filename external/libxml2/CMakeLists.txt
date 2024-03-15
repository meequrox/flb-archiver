include(FetchContent)

set(BUILD_SHARED_LIBS OFF)
set(LIBXML2_WITH_DEBUG OFF)
set(LIBXML2_WITH_PROGRAMS OFF)
set(LIBXML2_WITH_PYTHON OFF)
set(LIBXML2_WITH_TESTS OFF)
set(LIBXML2_WITH_ICONV OFF)
set(LIBXML2_WITH_LZMA OFF)
set(LIBXML2_WITH_ZLIB OFF)
set(LIBXML2_WITH_SAX1 OFF)

# https://github.com/GNOME/libxml2/tags
set(libxml2_version 2.12.6)
set(libxml2_sha256 b6ce34641cc2f3084289d5ac3cd49e8892394591a3df8c05f19d633f4e6058bc)

FetchContent_Declare(libxml2
  URL https://github.com/GNOME/libxml2/archive/refs/tags/v${libxml2_version}.tar.gz
  URL_HASH SHA256=${libxml2_sha256}
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}
)

FetchContent_MakeAvailable(libxml2)
