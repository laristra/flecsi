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

# Do we really need this? For some version (8.1.0) of vtk, the -std=c++11
# flag will be passed no mater how the user configure the CMAKE_CXX_FLAG
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
target_link_libraries(${TARGET} FleCSI ${FLECSI_LIBRARIES})
target_compile_features(${TARGET} PUBLIC cxx_std_14)

# make install strips RPATH without this.
set_target_properties(${TARGET} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

install(TARGETS ${TARGET} DESTINATION ${INSTALL_PREFIX})
""")
