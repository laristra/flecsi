# -*- CMake -*-

project(flecstan LANGUAGES CXX)
cmake_minimum_required(VERSION 3.8)

# Version
# fixme Reformulate for newer cmake
set(flecstan_VERSION_MAJOR 0)
set(flecstan_VERSION_MINOR 0)
set(flecstan_VERSION_PATCH 0)
configure_file (
  "src/flecstan-config.h.in"
  "src/flecstan-config.h"
)

# Use cmake modules from cinch
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cinch/cmake)

# C++
# fixme Replace hardcoded flags with something better
set(CMAKE_CXX_STANDARD 17)
if (NOT CMAKE_CXX_FLAGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -O2 -s")
endif()

# Packages
find_package(LLVM REQUIRED)
find_package(Clang CONFIG COMPONENTS Tooling QUIET)
if (NOT Clang_FOUND)
  find_package(Clang 7 MODULE REQUIRED COMPONENTS
    Tooling
    ASTMatchers
    Format
    ToolingInclusions
    Frontend
    Driver
    Parse
    Serialization
    Sema
    Edit
    Analysis
    ToolingCore
    AST
    Rewrite
    Lex
    Basic)
else()
  # not set by config, but set by module mode
  set(CLANG_LIBRARIES clangTooling)
endif()

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
  flecstan ${CLANG_LIBRARIES}
)
