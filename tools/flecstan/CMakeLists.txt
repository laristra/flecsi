project(flecstan LANGUAGES CXX)
cmake_minimum_required(VERSION 3.8)

# C++
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -pedantic -Wall -s")

# Packages
find_package(Clang   REQUIRED)
find_package(LLVM    REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB    REQUIRED)

# Include directories
include_directories(src)

# Source files
set(flecstan_SOURCES
   src/flecstan-analysis.cc
   src/flecstan-arg.cc
   src/flecstan-diagnostic.cc
   src/flecstan-macro.cc
   src/flecstan-main.cc
   src/flecstan-misc.cc
   src/flecstan-prep.cc
   src/flecstan-utility.cc
   src/flecstan-visitor.cc
   src/flecstan-yaml.cc
)

# Executable
add_executable(flecstan ${flecstan_SOURCES})

# Libraries
target_link_libraries(
   flecstan
   ${CMAKE_THREAD_LIBS_INIT}
   ${ZLIB_LIBRARIES}
   clangAnalysis clangAST clangBasic clangDriver clangEdit clangFrontend
   clangLex clangParse clangSema clangSerialization clangTooling
)
