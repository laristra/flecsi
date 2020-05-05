cmake_minimum_required(VERSION 3.12)
project(control-overview LANGUAGES CXX C)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(FleCSI REQUIRED)
include_directories(${FleCSI_INCLUDE_DIRS})

add_executable(control-overview control-overview.cc control-overview.hh)
target_link_libraries(control-overview ${FleCSI_LIBRARIES}
  ${FleCSI_LIBRARY_DEPENDENCIES})
