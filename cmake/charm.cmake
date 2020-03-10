#------------------------------------------------------------------------------#
#  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
# /@@/////  /@@          @@////@@ @@////// /@@
# /@@       /@@  @@@@@  @@    // /@@       /@@
# /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
# /@@////   /@@/@@@@@@@/@@       ////////@@/@@
# /@@       /@@/@@//// //@@    @@       /@@/@@
# /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
# //       ///  //////   //////  ////////  //
#
# Copyright (c) 2016 Los Alamos National Laboratory, LLC
# All rights reserved
#------------------------------------------------------------------------------#

option(ENABLE_CHARM "Enable Charm" OFF)

if(ENABLE_CHARM)

  find_package(Legion REQUIRED)

  if(NOT Legion_FOUND)
    message(FATAL_ERROR "Legion is required for this build configuration")
  endif(NOT Legion_FOUND)

  set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} ${LEGION_INSTALL_DIRS})

  include_directories(SYSTEM ${Legion_INCLUDE_DIRS})

  add_definitions(-DLEGION_USE_CMAKE)
  add_definitions(-DREALM_USE_CMAKE)

  list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${Legion_LIBRARIES})

  file(GLOB_RECURSE ci-files ${CMAKE_SOURCE_DIR}/flecsi/*.ci)

  foreach(in_file ${ci-files})
    get_filename_component(ci-output ${in_file} NAME_WE)
    get_filename_component(ci-dir ${in_file} DIRECTORY)
    string(APPEND ci-output ".decl.h")
    set(all-ci-outputs ${all-cioutputs} ${ci-dir}/${ci-output})
    add_custom_command(
      OUTPUT ${ci-dir}/${ci-output}
      COMMAND ${CMAKE_CXX_COMPILER} ${in_file}
      WORKING_DIRECTORY ${ci-dir}
      DEPENDS ${in_file}
    )
  endforeach()
  message (STATUS "Created command for " ${all-ci-outputs})

endif(ENABLE_CHARM)
