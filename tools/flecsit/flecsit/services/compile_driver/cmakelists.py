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

# This is needed to correctly handle flecsi-clang++ files with
# .fcc suffix. It should have no effect on normal C++ driver files.
set_source_files_properties(${DRIVER} PROPERTIES LANGUAGE CXX)

add_executable(${TARGET}
  ${DRIVER}
  ${FLECSI_RUNTIME_DRIVER}
  ${FLECSI_RUNTIME_MAIN}
)

target_compile_definitions(${TARGET} PRIVATE ${FLECSI_DEFINES})
target_link_libraries(${TARGET} FleCSI ${FLECSI_LIBRARIES})

install(TARGETS ${TARGET} DESTINATION ${INSTALL_PREFIX})
""")
