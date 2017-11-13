#~----------------------------------------------------------------------------~#
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
#~----------------------------------------------------------------------------~#

#------------------------------------------------------------------------------#
# Compile definition to include driver
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "legion" AND ENABLE_COLORING)

  add_definitions(-DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT)
  add_definitions(-DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER)

  #----------------------------------------------------------------------------#
  # Add source files for example application.
  #----------------------------------------------------------------------------#

  set(exec_path ${PROJECT_SOURCE_DIR}/flecsi/execution)
  set (exmaple_src      lax_wendroff.cc
                        ${exec_path}/legion/runtime_driver.cc)

  #----------------------------------------------------------------------------#
  # Add a rule to build the executable
  #----------------------------------------------------------------------------#


  add_executable(lax_wendroff ${exmaple_src})

  #----------------------------------------------------------------------------#
  # Add link dependencies
  #----------------------------------------------------------------------------#

  target_link_libraries(lax_wendroff FleCSI ${FLECSI_RUNTIME_LIBRARIES})

  configure_file(simple2d-32x32.msh
    ${CMAKE_CURRENT_BINARY_DIR}/../../examples/lax_wendroff/simple2d-32x32.msh COPYONLY) 

  configure_file(plot_lax.py
    ${CMAKE_CURRENT_BINARY_DIR}/../../examples/lax_wendroff/plot_lax.py COPYONLY) 


endif()
#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
