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
# Add header files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(execution_HEADERS
  common/function_handle.h
  common/launch.h
  common/processor.h
  context.h
  default_driver.h
  execution.h
  function.h
  kernel.h
  task.h
)

set(DRIVER_INITIALIZATION driver_initialization.cc)

#------------------------------------------------------------------------------#
# Runtime-specific files.
#
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
  set(execution_SOURCES
    ${execution_SOURCES}
    serial/execution_policy.cc
    )
  set(UNIT_POLICY SERIAL)
  set(RUNTIME_DRIVER serial/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(execution_HEADERS
    ${execution_HEADERS}
    legion/context_policy.h
    legion/execution_policy.h
    legion/future.h
    legion/helper.h
    legion/internal_task.h
    legion/legion_tasks.h
    legion/mapper.h
    legion/registration_wrapper.h
    legion/runtime_driver.h
    legion/runtime_state.h
    legion/task_wrapper.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    legion/context_policy.cc
    legion/execution_policy.cc
    legion/legion_tasks.cc
  )

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER legion/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(execution_HEADERS
    ${execution_HEADERS}
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    mpi/context_policy.cc
  )

  set(UNIT_POLICY MPI)
  set(RUNTIME_DRIVER mpi/runtime_driver.cc)

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(execution_HEADERS
  ${execution_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

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

cinch_add_devel_target(devel
  SOURCES
    test/devel.cc
  POLICY
    ${UNIT_POLICY}
)

if((NOT DEFINED ENV{TRAVIS} ) OR (FLECSI_RUNTIME_MODEL STREQUAL "serial")
    OR (FLECSI_RUNTIME_MODEL STREQUAL "legion"))

  #
  # Test basic function calling capability using the register_function
  # interface.
  #
  cinch_add_unit(simple_function
    SOURCES
      test/simple_function.cc
      ${DRIVER_INITIALIZATION}
      ${RUNTIME_DRIVER}
    DEFINES
      -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
    POLICY
      ${UNIT_POLICY}
    LIBRARIES
      flecsi
  )

  #
  # Test basic task calling capability using the register_task
  # interface.
  #
  cinch_add_unit(simple_task
    SOURCES
      test/simple_task.cc
      ${DRIVER_INITIALIZATION}
      ${RUNTIME_DRIVER}
    DEFINES
      -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
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
    cinch_add_unit(internal_task
    SOURCES
      test/legion/internal_task_driver.cc
      ${DRIVER_INITIALIZATION}
      ${RUNTIME_DRIVER}
    DEFINES
      -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
    POLICY
      ${UNIT_POLICY}
    LIBRARIES
      flecsi
      ${CINCH_RUNTIME_LIBRARIES}
    THREADS 2
    )

    if(ENABLE_COLORING AND ENABLE_PARMETIS)

      cinch_add_devel_target(execution_structure
        SOURCES
          test/execution_structure.cc
          ../supplemental/coloring/add_colorings.cc
          ${DRIVER_INITIALIZATION}
          ${RUNTIME_DRIVER}
        INPUTS
          test/simple2d-8x8.msh
          test/simple2d-16x16.msh
        LIBRARIES
          flecsi
          ${CINCH_RUNTIME_LIBRARIES}
          ${COLORING_LIBRARIES}
        DEFINES
          -DFLECSI_ENABLE_SPECIALIZATION_DRIVER
          -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
        POLICY LEGION
      )

      cinch_add_unit(ghost_access
        SOURCES
          test/legion/ghost_access_drivers.cc
          ../supplemental/coloring/add_colorings.cc
          ${DRIVER_INITIALIZATION}
          ${RUNTIME_DRIVER}
        INPUTS
          test/simple2d-8x8.msh
          test/simple2d-16x16.msh
        LIBRARIES
          flecsi
          ${CINCH_RUNTIME_LIBRARIES}
          ${COLORING_LIBRARIES}
        DEFINES
          -DFLECSI_ENABLE_SPECIALIZATION_DRIVER
          -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
        POLICY LEGION
        THREADS 2
      )

      cinch_add_unit(data_handle_task
        SOURCES
          test/data_handle_task.cc
          ../supplemental/coloring/add_colorings.cc
          ${DRIVER_INITIALIZATION}
          ${RUNTIME_DRIVER}
        INPUTS
          test/simple2d-8x8.msh
          test/simple2d-16x16.msh
        LIBRARIES
          flecsi
          ${CINCH_RUNTIME_LIBRARIES}
          ${COLORING_LIBRARIES}
        DEFINES
          -DFLECSI_ENABLE_SPECIALIZATION_DRIVER
          -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
        POLICY LEGION
        THREADS 2
      )

    endif()

  endif()

endif()

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
