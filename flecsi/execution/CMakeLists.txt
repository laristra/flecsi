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
  common/function_handle.h
  common/launch.h
  common/processor.h
  common/task_hash.h
  context.h
  execution.h
  function.h
  kernel.h
  task.h
)

#------------------------------------------------------------------------------#
# UNIT_POLICY and RUNTIME_DRIVER are set for unit tests that are not
# runtime specific and can be configured for whichever runtime is active.
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  set(execution_HEADERS
    ${execution_HEADERS}
    serial/context_policy.h
    serial/execution_policy.h
    serial/runtime_driver.h
  )

  set(UNIT_POLICY SERIAL)
  set(RUNTIME_DRIVER serial/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(execution_HEADERS
    ${execution_HEADERS}
    legion/context_policy.h
    legion/execution_policy.h
    legion/future.h
    legion/handshake.h
    legion/helper.h
    legion/internal_task.h
    legion/mpi_legion_interop.h
    legion/runtime_driver.h
    legion/task_args.h
    legion/task_wrapper.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    ../data/legion/dpd.cc
    legion/context_policy.cc
  )

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER legion/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(execution_HEADERS
    ${execution_HEADERS}
  )

  set(execution_SOURCES
    ${execution_SOURCES}
  )

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER mpi/runtime_driver.cc)

endif()

# Capture header list at call site
set(execution_HEADERS
  ${execution_HEADERS}
  PARENT_SCOPE
)

# Capture source list at call site
set(execution_SOURCES
  ${execution_SOURCES}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

cinch_add_unit(launch
  SOURCES
    test/launch.cc
  POLICY
    SERIAL
)

cinch_add_unit(processor
  SOURCES
    test/processor.cc
    ${RUNTIME_DRIVER}
  POLICY
    SERIAL
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
)

cinch_add_unit(task_hash
  SOURCES
    test/task_hash.cc
  POLICY
    SERIAL
)

if((NOT DEFINED ENV{TRAVIS} ) OR (FLECSI_RUNTIME_MODEL STREQUAL "serial")
    OR (FLECSI_RUNTIME_MODEL STREQUAL "legion"))

#
# Test basic function calling capability using the register_function
# interface.
#
cinch_add_unit(simple_function
  SOURCES
    test/simple_function_driver.cc
    test/context_runtime.cc
    ${RUNTIME_DRIVER}
  POLICY
    ${UNIT_POLICY}
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
)

#
# Test basic task calling capability using the register_task
# interface.
#
cinch_add_unit(simple_task
  SOURCES
    test/simple_task_driver.cc
    test/context_runtime.cc
    ${RUNTIME_DRIVER}
  POLICY
    ${UNIT_POLICY}
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
)
  if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

    #
    # Test internal legion task registration.
    #
#    cinch_add_unit(internal_task
#    SOURCES
#      test/legion/internal_task_driver.cc
#      test/context_runtime.cc
#      ${RUNTIME_DRIVER}
#    POLICY
#      ${UNIT_POLICY}
#    LIBRARIES
#      flecsi
#      ${CINCH_RUNTIME_LIBRARIES}
#    )

  endif()

endif()

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
