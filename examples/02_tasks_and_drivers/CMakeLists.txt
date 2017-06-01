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

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  add_definitions(-DFLECSI_OVERRIDE_DEFAULT_SPECIALIZATION_DRIVER)
  add_definitions(-DFLECSI_ENABLE_SPECIALIZATION_DRIVER)

  #----------------------------------------------------------------------------#
  # Add source files for example application.
  #----------------------------------------------------------------------------#

  set(exec_path ${PROJECT_SOURCE_DIR}/flecsi/execution)
  set (exmaple_src      tasks_and_drivers.cc
                        ${exec_path}/legion/runtime_driver.cc)

  #----------------------------------------------------------------------------#
  # Add a rule to build the executable
  #----------------------------------------------------------------------------#


  add_executable(tasks_and_drivers ${exmaple_src})

  #----------------------------------------------------------------------------#
  # Add link dependencies
  #----------------------------------------------------------------------------#

  target_link_libraries(tasks_and_drivers flecsi ${FLECSI_RUNTIME_LIBRARIES})

endif()
#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
