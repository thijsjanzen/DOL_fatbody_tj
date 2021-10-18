cmake_minimum_required (VERSION 3.15)

project("DOL_fatbody" CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_STANDARD 17)

if(WIN32)
  # required, config_parser.h will break things on Windows
  add_definitions(/DNOMINMAX /DWIN32_LEAN_AND_MEAN /DNOGDI)
endif()

# too buggy to be worth it
# add_subdirectory(test ${CMAKE_SOURCE_DIR}/bin/)

add_executable(fatbody "main.cpp" )

# move to a common place
install(TARGETS fatbody RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin)
