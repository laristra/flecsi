#------------------------------------------------------------------------------#
# Copyright (c) 2016, Triad National Security, LLC
# All rights reserved
#------------------------------------------------------------------------------#

cmake_minimum_required(VERSION 3.12)

#------------------------------------------------------------------------------#
# Set the project name.
#------------------------------------------------------------------------------#

project(standalone LANGUAGES CXX C)

#------------------------------------------------------------------------------#
# FleCSI currently depends on C++17.
#------------------------------------------------------------------------------#

set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

#------------------------------------------------------------------------------#
# Find the FleCSI installation.
#
# The easiest way to help CMake find FleCSI is to install it,
# and add the location to the CMAKE_PREFIX_PATH environment variable.
#------------------------------------------------------------------------------#

find_package(FleCSI REQUIRED)
include_directories(${FleCSI_INCLUDE_DIRS})

#------------------------------------------------------------------------------#
# App
#------------------------------------------------------------------------------#

add_subdirectory(app)
