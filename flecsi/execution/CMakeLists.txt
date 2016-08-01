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

set(execution_HEADERS
  context.h
  task.h
  default/default_driver.h
  default/default_execution_policy.h
  PARENT_SCOPE
)

set(execution_SOURCES
  legion/legion_runtime_driver.cc
  PARENT_SCOPE
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")
  cinch_add_unit(legion
    SOURCES test/legion.cc
    POLICY LEGION
    LIBRARIES flecsi)
endif()

#cinch_add_unit(mpi
#  SOURCES test/mpi.cc
#  POLICY MPI
#  THREADS 2)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
