#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

from string import Template

cmakelists_template = Template(
"""
#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED})

project(${PROJECT})

find_package(FleCSI REQUIRED)

include_directories($${FLECSI_INCLUDE_DIRS})

add_executable(${TARGET}
  ${DRIVER}
  ${FLECSI_RUNTIME_DRIVER}
  ${FLECSI_RUNTIME_MAIN}
)

target_link_libraries(${TARGET} FleCSI)

install(TARGETS ${TARGET} DESTINATION ${INSTALL_PREFIX})
""")
