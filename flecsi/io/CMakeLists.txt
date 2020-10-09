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

set(io_HEADERS
  backend.h
  io.h
  io_base.h
  io_interface.h
  simple_definition.h
)

set(io_SOURCES
)

if(ENABLE_HDF5)

  set(io_HEADERS
    io_hdf5.h
    ${io_HEADERS}
  )

endif() # ENABLE_HDF5

set(DRIVER_INITIALIZATION ../execution/driver_initialization.cc)

#------------------------------------------------------------------------------#
# Runtime-specific files.
#
# UNIT_POLICY and RUNTIME_DRIVER are set for unit tests that are not
# runtime specific and can be configured for whichever runtime is active.
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "mpi")
  set(io_HEADERS
    mpi/policy.h
    ${io_HEADERS}
  )
endif()

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  if(ENABLE_HDF5)
    set(io_HEADERS
      legion/policy.h
      ${io_HEADERS}
    )

    set(io_SOURCES
      legion/policy.cc
      ${io_SOURCES}
    )
  endif()

endif()

if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  set(UNIT_POLICY SERIAL)
  set(RUNTIME_DRIVER ../execution/serial/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER ../execution/legion/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(UNIT_POLICY MPI)
  set(RUNTIME_DRIVER ../execution/mpi/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(UNIT_POLICY HPX)
  set(RUNTIME_DRIVER hpx/runtime_driver.cc)

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(io_HEADERS
    ${io_HEADERS}
    PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(io_SOURCES
    ${io_SOURCES}
    PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

# TODO:  Test that we have parallel HDF5
if(FLECSI_RUNTIME_MODEL STREQUAL "mpi" AND ENABLE_HDF5)
  cinch_add_unit(hdf5_restart
    SOURCES
      test/hdf5_restart.cc
      ../supplemental/coloring/add_colorings.cc
      ${DRIVER_INITIALIZATION}
      ${RUNTIME_DRIVER}
    INPUTS
      test/simple2d-16x16.msh
    LIBRARIES
      FleCSI
      ${CINCH_RUNTIME_LIBRARIES}
      ${COLORING_LIBRARIES}
      ${HDF5_LIBRARIES}
    DEFINES
      -DFLECSI_ENABLE_SPECIALIZATION_TLT_INIT
      -DFLECSI_ENABLE_SPECIALIZATION_SPMD_INIT
      -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
      -DFLECSI_16_16_MESH
    POLICY ${UNIT_POLICY}
    THREADS 4
  )
endif()

if(FLECSI_RUNTIME_MODEL STREQUAL "legion" AND ENABLE_HDF5)

  cinch_add_unit(io_hdf5
    SOURCES
      test/legion/io_hdf5.cc
      ${DRIVER_INITIALIZATION}
      ${RUNTIME_DRIVER}
    DEFINES
      -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
    POLICY
      ${UNIT_POLICY}
    LIBRARIES
      FleCSI
      ${CINCH_RUNTIME_LIBRARIES}
      ${HDF5_LIBRARIES}
    THREADS 4
  )
endif()

cinch_add_unit(simple_definition
  SOURCES test/simple_definition.cc
  INPUTS test/simple2d-8x8.msh test/simple2d-4x4.msh
)

set(io_HEADERS
  ${io_HEADERS}
  io_exodus.h
  PARENT_SCOPE
)

cinch_add_unit(io
  SOURCES test/io.cc
)
