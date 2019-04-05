# -*- CMake -*-

project(flecstan LANGUAGES CXX)
cmake_minimum_required(VERSION 3.8)

# Use cmake modules from cinch
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cinch/cmake)

# C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -O2 -s")

# Packages
find_package(LLVM    REQUIRED)
find_package(Clang   REQUIRED COMPONENTS Analysis AST Basic Driver Edit Frontend Lex Parse Sema Serialization Tooling)
find_package(Threads REQUIRED)
find_package(ZLIB    REQUIRED)

# Include directories
include_directories(src ${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})

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
   ${CLANG_LIBRARIES}
   ${LLVM_LIBRARIES}
)
