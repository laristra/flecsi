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

set(coloring_HEADERS
  adjacency_types.h
  colorer.h
  coloring_types.h
  communicator.h
  crs.h
  index_coloring.h
  dcrs_utils.h 
  mpi_communicator.h
  mpi_utils.h
  parmetis_colorer.h
)

#------------------------------------------------------------------------------#
# Runtime-specific files.
#
# UNIT_POLICY and RUNTIME_DRIVER are set for unit tests that are not
# runtime specific and can be configured for whichever runtime is active.
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  set(UNIT_POLICY SERIAL)
  set(RUNTIME_DRIVER ../execution/serial/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER ../execution/legion/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(UNIT_POLICY MPI)
  set(RUNTIME_DRIVER ../execution/mpi/runtime_driver.cc)

endif()

#------------------------------------------------------------------------------#
# Parallel library support.
#------------------------------------------------------------------------------#

if(ENABLE_MPI)
  set(coloring_HEADERS
    ${coloring_HEADERS}
    dcrs_utils.h
    mpi_communicator.h
    mpi_utils.h
  )
endif()

if(ENABLE_PARMETIS)
  set(coloring_HEADERS
    ${coloring_HEADERS}
    parmetis_colorer.h
  )
endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(coloring_HEADERS
    ${coloring_HEADERS}
    PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(coloring_SOURCES
    ${coloring_SOURCES}
    PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

cinch_add_unit(dcrs
  SOURCES test/dcrs.cc
  INPUTS
    test/simple2d-8x8.msh
    test/simple2d-16x16.msh
  LIBRARIES ${COLORING_LIBRARIES}
  POLICY MPI
  THREADS 5
)

cinch_add_devel_target(devel-dcrs
  SOURCES test/devel-dcrs.cc
  INPUTS
    test/simple2d-8x8.msh
    test/simple2d-16x16.msh
  LIBRARIES ${COLORING_LIBRARIES}
  POLICY MPI
  THREADS 5
)

# Both of these tests depend on ParMETIS.
# This could change if we add more colorer types.
if(ENABLE_COLORING AND ENABLE_PARMETIS)

cinch_add_devel_target(coloring
  SOURCES test/coloring.cc
  INPUTS
    test/simple2d-8x8.msh
    test/simple2d-16x16.msh
    test/simple2d-32x32.msh
    test/simple2d-48x48.msh
  LIBRARIES ${COLORING_LIBRARIES}
  POLICY MPI
  THREADS 5
)

endif()
