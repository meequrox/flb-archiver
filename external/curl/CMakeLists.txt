include(FetchContent)

set(BUILD_CURL_EXE OFF)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_STATIC_LIBS ON)
set(CURL_DISABLE_TESTS ON)
set(CURL_LTO ON)

set(CURL_USE_ZLIB OFF)
set(CURL_USE_LIBPSL OFF)
set(CURL_USE_LIBSSH2 OFF)
set(HTTP_ONLY ON)
set(CURL_USE_SCHANNEL ON)

# https://github.com/curl/curl/releases
set(curl_version_ 8_6_0)
set(curl_version 8.6.0)
set(curl_sha256 9c6db808160015f30f3c656c0dec125feb9dc00753596bf858a272b5dd8dc398)

FetchContent_Declare(curl
  URL https://github.com/curl/curl/releases/download/curl-${curl_version_}/curl-${curl_version}.tar.gz
  URL_HASH SHA256=${curl_sha256}
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}
)

FetchContent_MakeAvailable(curl)
