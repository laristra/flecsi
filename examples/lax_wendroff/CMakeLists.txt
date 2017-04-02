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

if(FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")

if(ENABLE_PARTITIONING AND ENABLE_PARMETIS)

#  set(CMAKE_CXX_COMPILER "mpiCC")

  add_definitions(-DFLECSI_OVERRIDE_DEFAULT_SPECIALIZATION_DRIVER)

  #----------------------------------------------------------------------------#
  # Add a rule to build the executable
  #----------------------------------------------------------------------------#

  set(exec_path ${PROJECT_SOURCE_DIR}/flecsi/execution)
  set (exmaple_src      lax_wendroff.cc
                        lax_wendroff_driver.cc
                        ${exec_path}/mpilegion/runtime_driver.cc
                        ${exec_path}/../data/data_client.cc
                        ${exec_path}/mpilegion/copy_ghosts.cc
                        ${exec_path}/test/mpilegion/init_partitions_task.cc
                        ${exec_path}/../data/legion/dpd.cc)

  add_executable(lax_wendroff ${exmaple_src})
  set_target_properties(lax_wendroff PROPERTIES LINKER_LANGUAGE CXX)

  #----------------------------------------------------------------------------#
  # Add link dependencies
  #----------------------------------------------------------------------------#

  target_link_libraries(lax_wendroff ${FLECSI_RUNTIME_LIBRARIES}
                                     ${PARTITION_LIBRARIES})

  configure_file(simple2d-32x32.msh
    ${CMAKE_CURRENT_BINARY_DIR}/../../bin/simple2d-32x32.msh COPYONLY) 

  configure_file(plot_lax.py
    ${CMAKE_CURRENT_BINARY_DIR}/../../bin/plot_lax.py COPYONLY) 
 
endif()

endif()
#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
