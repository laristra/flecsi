#------------------------------------------------------------------------------#
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
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
# Add header files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(execution_HEADERS
  common/function_handle.h
  common/launch.h
  common/processor.h
  common/execution_state.h
  context.h
  default_driver.h
  execution.h
  function.h
  global_object_wrapper.h
  internal_index_space.h
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

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(execution_HEADERS
    ${execution_HEADERS}
    legion/context_policy.h
    legion/execution_policy.h
    legion/future.h
    legion/helper.h
    legion/init_args.h
    legion/init_handles.h
    legion/internal_field.h
    legion/internal_task.h
    legion/legion_tasks.h
    legion/mapper.h
    legion/finalize_handles.h
    legion/registration_wrapper.h
    legion/runtime_driver.h
    legion/runtime_state.h
    legion/task_epilog.h
    legion/task_prolog.h
    legion/task_wrapper.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    legion/context_policy.cc
    legion/legion_tasks.cc
  )

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER legion/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(execution_HEADERS
    ${execution_HEADERS}
    mpi/context_policy.h
    mpi/execution_policy.h
    mpi/finalize_handles.h
    mpi/future.h
    mpi/reduction_wrapper.h
    mpi/runtime_driver.h
    mpi/task_epilog.h
    mpi/task_prolog.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    mpi/context_policy.cc
  )

  set(UNIT_POLICY MPI)
  set(RUNTIME_DRIVER mpi/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(execution_HEADERS
    ${execution_HEADERS}
    hpx/context_policy.h
    hpx/execution_policy.h
    hpx/future.h
    hpx/runtime_driver.h
  )
  set(execution_SOURCES
    ${execution_SOURCES}
    hpx/context_policy.cc
    )
  set(UNIT_POLICY HPX)
  set(RUNTIME_DRIVER hpx/runtime_driver.cc)

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
  FOLDER
    "Tests/Execution"
)

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
    FleCSI
  NOCI
  FOLDER
    "Tests/Execution"
  )

#
# Test global reduction
#

cinch_add_unit(reduction
  SOURCES
    test/reduction.cc
    ${DRIVER_INITIALIZATION}
    ${RUNTIME_DRIVER}
  DEFINES
    -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
  POLICY
    ${UNIT_POLICY}
  LIBRARIES
    FleCSI
  THREADS 2
  NOCI
  FOLDER
    "Tests/Execution"
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
    FleCSI
    ${CINCH_RUNTIME_LIBRARIES}
  NOCI
  FOLDER
    "Tests/Execution"
  )

if(NOT FLECSI_RUNTIME_MODEL STREQUAL "hpx")
#
# Test basic task to test future_handle
#
  cinch_add_unit(future_handle
    SOURCES
      test/future_handle.cc
      ${DRIVER_INITIALIZATION}
      ${RUNTIME_DRIVER}
    DEFINES
      -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
      -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
    POLICY
      ${UNIT_POLICY}
    LIBRARIES
      FleCSI
      ${CINCH_RUNTIME_LIBRARIES}
    THREADS 2
    NOCI
    )

  if(ENABLE_PARMETIS)

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
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
        ${COLORING_LIBRARIES}
      DEFINES
        -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
        -DFLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
        -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
        -DFLECSI_8_8_MESH
      POLICY ${UNIT_POLICY}
    )

    cinch_add_devel_target(devel_handle
      SOURCES
        test/devel_handle.cc
        ../supplemental/coloring/add_colorings.cc
        ${DRIVER_INITIALIZATION}
        ${RUNTIME_DRIVER}
      INPUTS
        test/simple2d-8x8.msh
        test/simple2d-16x16.msh
      LIBRARIES
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
        ${COLORING_LIBRARIES}
      DEFINES
        -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
        -DFLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
        -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
        -DFLECSI_16_16_MESH
      POLICY ${UNIT_POLICY}
    )

    cinch_add_devel_target(concept_coloring
      SOURCES
        test/concept_coloring.cc
        ../supplemental/coloring/concept_coloring.cc
        ${DRIVER_INITIALIZATION}
        ${RUNTIME_DRIVER}
      INPUTS
        test/simple2d-8x8.msh
        test/simple2d-16x16.msh
      LIBRARIES
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
        ${COLORING_LIBRARIES}
      DEFINES
        -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
        -DFLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
        -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
        -DFLECSI_16_16_MESH
      POLICY ${UNIT_POLICY}
    )

    cinch_add_devel_target(index_subspaces
      SOURCES
        test/index_subspaces.cc
        ../supplemental/coloring/add_colorings.cc
        ${DRIVER_INITIALIZATION}
        ${RUNTIME_DRIVER}
      INPUTS
        test/simple2d-8x8.msh
        test/simple2d-16x16.msh
      LIBRARIES
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
        ${COLORING_LIBRARIES}
      DEFINES
        -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
        -DFLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
        -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
        -DFLECSI_8_8_MESH
      POLICY ${UNIT_POLICY}
    )

    # cinch_add_unit(forall
    #   SOURCES
    #     test/forall.fcc
    #     ../supplemental/coloring/add_colorings.cc
    #     ${DRIVER_INITIALIZATION}
    #     ${RUNTIME_DRIVER}
    #   INPUTS
    #     test/simple2d-8x8.msh
    #     test/simple2d-16x16.msh
    #   LIBRARIES
    #     FleCSI
    #     ${CINCH_RUNTIME_LIBRARIES}
    #     ${COLORING_LIBRARIES}
    #   DEFINES
    #     -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
    #     -DFLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
    #     -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
    #     -DFLECSI_8_8_MESH
    #   POLICY ${UNIT_POLICY}
    #   NOCI
    # )

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
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
        ${COLORING_LIBRARIES}
      DEFINES
        -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
        -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
      POLICY ${UNIT_POLICY}
      THREADS 2
      NOCI
    )

    cinch_add_unit(handle_accessor
      SOURCES
        test/handle_accessor.cc
        ../supplemental/coloring/add_colorings.cc
        ${DRIVER_INITIALIZATION}
        ${RUNTIME_DRIVER}
      INPUTS
        test/simple2d-8x8.msh
        test/simple2d-16x16.msh
      LIBRARIES
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
        ${COLORING_LIBRARIES}
      DEFINES
        -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
        -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
      POLICY ${UNIT_POLICY}
      THREADS 2
      NOCI
    )

    cinch_add_unit(ghost_access
      SOURCES
        test/ghost_access_drivers.cc
        ../supplemental/coloring/add_colorings.cc
        ${DRIVER_INITIALIZATION}
        ${RUNTIME_DRIVER}
      INPUTS
        test/simple2d-8x8.msh
        test/simple2d-16x16.msh
      LIBRARIES
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
        ${COLORING_LIBRARIES}
      DEFINES
        -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
        -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
      POLICY ${UNIT_POLICY}
      THREADS 2
      NOCI
    )

    cinch_add_unit(unordered_ispaces
      SOURCES
        test/unordered_ispaces.cc
        ${DRIVER_INITIALIZATION}
        ${RUNTIME_DRIVER}
      INPUTS
        test/simple2d-8x8.msh
        test/simple2d-16x16.msh
      LIBRARIES
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
        ${COLORING_LIBRARIES}
      DEFINES
        -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
        -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
      POLICY ${UNIT_POLICY}
      NOCI
    )

    cinch_add_unit(data_client_handle
      SOURCES
        test/data_client_handle.cc
        ../supplemental/coloring/add_colorings.cc
        ${DRIVER_INITIALIZATION}
        ${RUNTIME_DRIVER}
      INPUTS
        test/simple2d-8x8.msh
        test/simple2d-16x16.msh
        test/data_client_handle.blessed
      LIBRARIES
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
        ${COLORING_LIBRARIES}
      DEFINES
        -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
        -DFLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
        -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
        -DFLECSI_8_8_MESH
      POLICY ${UNIT_POLICY}
      THREADS 2
      NOCI
    )

    cinch_add_unit(dense_data
      SOURCES
        test/dense_data.cc
        ../supplemental/coloring/add_colorings.cc
        ${DRIVER_INITIALIZATION}
        ${RUNTIME_DRIVER}
      INPUTS
        test/simple2d-8x8.msh
        test/simple2d-16x16.msh
        test/dense_data.blessed
      LIBRARIES
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
        ${COLORING_LIBRARIES}
      DEFINES
        -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
        -DFLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
        -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
        -DFLECSI_8_8_MESH
      POLICY ${UNIT_POLICY}
      THREADS 2
      NOCI
    )

    cinch_add_unit(finite_difference_dense
    SOURCES
      test/finite_difference_dense.cc
      ../supplemental/coloring/add_colorings.cc
      ${DRIVER_INITIALIZATION}
      ${RUNTIME_DRIVER}
    INPUTS
      test/simple2d-8x8.msh
      test/simple2d-16x16.msh
    LIBRARIES
      FleCSI
      ${CINCH_RUNTIME_LIBRARIES}
      ${COLORING_LIBRARIES}
    DEFINES
      -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
      -DFLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
      -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
      -DFLECSI_16_16_MESH
    POLICY ${UNIT_POLICY}
    THREADS 5
    NOCI
  )

    cinch_add_unit(global_data
      SOURCES
        test/global_data.cc
        ${DRIVER_INITIALIZATION}
        ${RUNTIME_DRIVER}
      INPUTS
        test/global_data.blessed
      LIBRARIES
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
      DEFINES
        -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
        -DFLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
        -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
      POLICY ${UNIT_POLICY}
      THREADS 2
      NOCI
    )

    cinch_add_unit(sparse_data
      SOURCES
        test/sparse_data.cc
        ../supplemental/coloring/add_colorings.cc
        ${DRIVER_INITIALIZATION}
        ${RUNTIME_DRIVER}
      INPUTS
        test/simple2d-8x8.msh
        test/simple2d-16x16.msh
        test/sparse_data.blessed
      LIBRARIES
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
        ${COLORING_LIBRARIES}
      DEFINES
        -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
        -DFLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
        -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
        -DFLECSI_8_8_MESH
      POLICY ${UNIT_POLICY}
      THREADS 4
      NOCI
    )

    cinch_add_unit(finite_difference_sparse
      SOURCES
        test/finite_difference_sparse.cc
        ../supplemental/coloring/add_colorings.cc
        ${DRIVER_INITIALIZATION}
        ${RUNTIME_DRIVER}
      INPUTS
        test/simple2d-8x8.msh
        test/simple2d-16x16.msh
      LIBRARIES
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
        ${COLORING_LIBRARIES}
      DEFINES
        -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
        -DFLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
        -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
        -DFLECSI_16_16_MESH
      POLICY ${UNIT_POLICY}
      THREADS 5
      NOCI
    )

    cinch_add_unit(ragged_data
      SOURCES
        test/ragged_data.cc
        ../supplemental/coloring/add_colorings.cc
        ${DRIVER_INITIALIZATION}
        ${RUNTIME_DRIVER}
      INPUTS
        test/simple2d-8x8.msh
        test/simple2d-16x16.msh
      LIBRARIES
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
        ${COLORING_LIBRARIES}
      DEFINES
        -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
        -DFLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
        -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
        -DFLECSI_8_8_MESH
      POLICY ${UNIT_POLICY}
      THREADS 2
      NOCI
    )

    if(FLECSI_RUNTIME_MODEL STREQUAL "mpi")
      cinch_add_unit(set_topology
        SOURCES
          test/set_topology.cc
          ${DRIVER_INITIALIZATION}
          ${RUNTIME_DRIVER}
        LIBRARIES
            FleCSI
          ${CINCH_RUNTIME_LIBRARIES}
          ${COLORING_LIBRARIES}
        DEFINES
          -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
          -DFLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
          -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
        POLICY ${UNIT_POLICY}
        THREADS 2
        NOCI
      )

      # cinch_add_unit(particles
      #   SOURCES
      #     test/particles.cc
      #     ${DRIVER_INITIALIZATION}
      #     ${RUNTIME_DRIVER}
      #   LIBRARIES
      #       FleCSI
      #     ${CINCH_RUNTIME_LIBRARIES}
      #     ${COLORING_LIBRARIES}
      #   DEFINES
      #     -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
      #     -DFLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
      #     -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
      #   POLICY ${UNIT_POLICY}
      #   THREADS 2
      #   NOCI
      # )
    endif() # mpi

  endif() # (ENABLE_PARMETIS)

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
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
      THREADS 2
      NOCI
    )

    cinch_add_unit(mapper_compaction_task
      SOURCES
        test/legion/mapper_compaction.cc
        ${DRIVER_INITIALIZATION}
        ${RUNTIME_DRIVER}
      DEFINES
        -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
      POLICY
        ${UNIT_POLICY}
      LIBRARIES
        FleCSI
        ${CINCH_RUNTIME_LIBRARIES}
      THREADS 1
      NOCI
    )

    if(ENABLE_PARMETIS)

      cinch_add_unit(barrier_per_field
        SOURCES
          test/legion/barrier_per_field_drivers.cc
          ../supplemental/coloring/add_colorings.cc
          ${DRIVER_INITIALIZATION}
          ${RUNTIME_DRIVER}
        INPUTS
          test/simple2d-8x8.msh
          test/simple2d-16x16.msh
        LIBRARIES
          FleCSI
          ${CINCH_RUNTIME_LIBRARIES}
          ${COLORING_LIBRARIES}
        DEFINES
          -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
          -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
        POLICY LEGION
        THREADS 2
        NOCI
      )

      cinch_add_unit(gid_to_lid_map
        SOURCES
          test/gid_to_lid_map.cc
          ../supplemental/coloring/add_colorings.cc
          ${DRIVER_INITIALIZATION}
          ${RUNTIME_DRIVER}
        INPUTS
          test/simple2d-8x8.msh
          test/simple2d-16x16.msh
        LIBRARIES
          FleCSI
          ${CINCH_RUNTIME_LIBRARIES}
          ${COLORING_LIBRARIES}
        DEFINES
          -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
          -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
        POLICY LEGION
        THREADS 2
        NOCI
      )

    endif() # (ENABLE_PARMETIS)

  endif() # legion

endif()# not hpx
