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
  default_driver.h
  execution.h
  task.h
  serial/execution_policy.h
  serial/runtime_driver.h
  legion/context_policy.h
  legion/execution_policy.h
  legion/runtime_driver.h
  legion/task_wrapper.h
  PARENT_SCOPE
)

set(execution_SOURCES
    PARENT_SCOPE
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(execution_SOURCES ${execution_SOURCES}
    legion/runtime_driver.cc
    PARENT_SCOPE
  )

  cinch_add_unit(task
    SOURCES test/task.cc
    POLICY LEGION
    LIBRARIES flecsi)
else()
  set(execution_SOURCES ${execution_SOURCES}
    serial/runtime_driver.cc
    PARENT_SCOPE
  )

  cinch_add_unit(task
    SOURCES test/task.cc
    LIBRARIES flecsi)
endif()

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
