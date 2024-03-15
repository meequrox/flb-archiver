# set(FETCHCONTENT_QUIET FALSE)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

add_subdirectory(libxml2)

if(WIN32)
  add_subdirectory(curl)
endif()
