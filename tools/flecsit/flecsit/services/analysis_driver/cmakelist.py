#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

from string import Template

cmake_source_template = Template(
"""
#/*~-------------------------------------------------------------------------~~*
# * Copyright (c) 2014 Los Alamos National Security, LLC
# * All rights reserved.
# *~-------------------------------------------------------------------------~~*/

cmake_minimum_required(${CMAKE_VERSION})
project(${PROJECT_NAME})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_INCLUDE_DIRS})

add_definitions(${CMAKE_DEFINES})

add_definitions(-std=c++14)

add_executable(${PROJECT_NAME} 
  runtime_driver.cc 
  runtime_main.cc 
  ${PROJECT_NAME}.cc)
"""
)

#------------------------------------------------------------------------------#
# vim: set tabstop=2 shiftwidth=2 expandtab :
#------------------------------------------------------------------------------#
