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
set(curl_version_ 8_4_0)
set(curl_version 8.4.0)
set(curl_sha256 816e41809c043ff285e8c0f06a75a1fa250211bbfb2dc0a037eeef39f1a9e427)

FetchContent_Declare(curl
  URL https://github.com/curl/curl/releases/download/curl-${curl_version_}/curl-${curl_version}.tar.gz
  URL_HASH SHA256=${curl_sha256}
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}
)

FetchContent_MakeAvailable(curl)
