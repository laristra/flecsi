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
  default_execution_policy.h
  execution.h
  legion/legion_context_policy.h
  legion/legion_runtime_driver.h
  legion/legion_task_wrapper.h
  legion/legion_execution_policy.h
  PARENT_SCOPE
)

set(execution_SOURCES
  legion/legion_runtime_driver.cc
  PARENT_SCOPE
)

cinch_add_unit(task
  SOURCES test/task.cc)

#if(FLECSI_RUNTIME_MODEL STREQUAL "legion" OR
#  FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")
#  cinch_add_unit(legion_flecsi_example
#    SOURCES test/legion.cc
#    POLICY LEGION)
#endif()

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")
cinch_add_unit(new_legion
  SOURCES test/new_legion.cc
  POLICY LEGION
  LIBRARIES flecsi)
endif()

cinch_add_unit(mpi
  SOURCES test/mpi.cc
  POLICY MPI
  THREADS 2)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
