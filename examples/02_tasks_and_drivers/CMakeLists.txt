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


#  set(CMAKE_CXX_COMPILER "mpiCC")

  add_definitions(-DFLECSI_OVERRIDE_DEFAULT_SPECIALIZATION_DRIVER)
  add_definitions(-DFLECSI_ENABLE_SPECIALIZATION_DRIVER)

  #----------------------------------------------------------------------------#
  # Add a rule to build the executable
  #----------------------------------------------------------------------------#

  set(exec_path ${PROJECT_SOURCE_DIR}/flecsi/execution)
  set (exmaple_src      tasks_and_drivers.cc
                        ${exec_path}/legion/execution_policy.cc
                        ${exec_path}/legion/context_policy.cc
                        ${exec_path}/legion/runtime_driver.cc
                        ${exec_path}/../data/data_client.cc
                        ${exec_path}/../utils/debruijn.cc
                        ${RUNTIME_DRIVER})

  add_executable(tasks_and_drivers ${exmaple_src})
  set_target_properties(tasks_and_drivers PROPERTIES LINKER_LANGUAGE CXX)

  #----------------------------------------------------------------------------#
  # Add link dependencies
  #----------------------------------------------------------------------------#

  target_link_libraries(tasks_and_drivers ${FLECSI_RUNTIME_LIBRARIES})

endif()
#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
