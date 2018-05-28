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

if(FLECSI_RUNTIME_MODEL STREQUAL "legion" OR
   FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  add_definitions(-DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT)
  add_definitions(-DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER)

  #----------------------------------------------------------------------------#
  # Add source files for example application.
  #----------------------------------------------------------------------------#

  set(exec_path ${PROJECT_SOURCE_DIR}/flecsi/execution)
  if(FLECSI_RUNTIME_MODEL STREQUAL "legion")
    set (exmaple_src      tasks_and_drivers.cc
                          ${exec_path}/legion/runtime_driver.cc)
  elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")
    set (exmaple_src      tasks_and_drivers.cc
                          ${exec_path}/hpx/runtime_driver.cc)
  endif()

  #----------------------------------------------------------------------------#
  # Add a rule to build the executable
  #----------------------------------------------------------------------------#


  add_executable(tasks_and_drivers ${exmaple_src})

  #----------------------------------------------------------------------------#
  # Add link dependencies
  #----------------------------------------------------------------------------#

  target_link_libraries(tasks_and_drivers FleCSI ${FLECSI_RUNTIME_LIBRARIES})

  if(FLECSI_RUNTIME_MODEL STREQUAL "hpx")
    hpx_setup_target(tasks_and_drivers FOLDER "Examples")
  else()
    set_target_properties(tasks_and_drivers PROPERTIES FOLDER "Examples")
  endif()

endif()
#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
