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

  add_definitions(-DREALM_USE_CMAKE)

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
