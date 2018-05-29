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

if(FLECSI_RUNTIME_MODEL STREQUAL "hpx")
  find_package(HPX REQUIRED)
  include_directories($${HPX_INCLUDE_DIRS})
  target_link_libraries(${DRIVER} $${HPX_LIBRARY_DIR})
endif()

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")
  find_package(Legion REQUIRED)
  include_directories($${LEGION_INCLUDE_DIRS})
  target_link_libraries(${DRIVER} $${LEGION_LIBRARY_DIR})
endif()

include_directories($${FLECSI_INCLUDE_DIRS})

${REQUIRED_PACKAGES}

# This is needed to correctly handle flecsi-clang++ files with
# .fcc suffix. It should have no effect on normal C++ driver files.
set_source_files_properties(${DRIVER} PROPERTIES LANGUAGE CXX)

add_executable(${TARGET}
  ${DRIVER}
  ${FLECSI_RUNTIME_DRIVER}
  ${FLECSI_RUNTIME_MAIN}
)

target_compile_definitions(${TARGET} PRIVATE ${FLECSI_DEFINES})

if(FLECSI_ENABLE_BOOST_PROGRAM_OPTIONS)
  target_compile_definitions(${TARGET} PRIVATE -DENABLE_BOOST_PROGRAM_OPTIONS)
endif()

target_link_libraries(${TARGET} FleCSI ${FLECSI_LIBRARIES})

# make install strips RPATH without this.
set_target_properties(${TARGET} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

install(TARGETS ${TARGET} DESTINATION ${INSTALL_PREFIX})
""")
