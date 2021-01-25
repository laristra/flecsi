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
  box_types.h
  box_colorer.h
  simple_box_colorer.h
)

#------------------------------------------------------------------------------#
# Runtime-specific files.
#
# UNIT_POLICY and RUNTIME_DRIVER are set for unit tests that are not
# runtime specific and can be configured for whichever runtime is active.
#------------------------------------------------------------------------------#

set(UNIT_POLICY MPI)

if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

#  set(UNIT_POLICY SERIAL)
  set(RUNTIME_DRIVER ../execution/serial/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

#  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER ../execution/legion/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

#  set(UNIT_POLICY MPI)
  set(RUNTIME_DRIVER ../execution/mpi/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(UNIT_POLICY HPX)
  set(RUNTIME_DRIVER ../execution/hpx/runtime_driver.cc)

endif()

#------------------------------------------------------------------------------#
# Parallel library support.
#------------------------------------------------------------------------------#

if(ENABLE_MPI)
  set(coloring_HEADERS
    ${coloring_HEADERS}
    dcrs_utils.h
    mpi_communicator.h
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
  LIBRARIES
    ${CINCH_RUNTIME_LIBRARIES}
    ${COLORING_LIBRARIES}
  POLICY ${UNIT_POLICY}
  THREADS 5
)

cinch_add_devel_target(devel-dcrs
  SOURCES test/devel-dcrs.cc
  INPUTS
    test/simple2d-8x8.msh
    test/simple2d-16x16.msh
  LIBRARIES
    ${CINCH_RUNTIME_LIBRARIES}
    ${COLORING_LIBRARIES}
  POLICY ${UNIT_POLICY}
  THREADS 5
  FOLDER "Tests/Coloring/Devel"
)

cinch_add_unit(boxcolor2d
  SOURCES test/test_simple_box_colorer_2d.cc
  INPUTS
  LIBRARIES ${COLORING_LIBRARIES}
  POLICY ${UNIT_POLICY}
  THREADS 4
)
cinch_add_unit(boxcolor3d
  SOURCES test/test_simple_box_colorer_3d.cc
  INPUTS
  LIBRARIES ${COLORING_LIBRARIES}
  POLICY ${UNIT_POLICY}
  THREADS 4
)
# Both of these tests depend on ParMETIS.
# This could change if we add more colorer types.
if(ENABLE_PARMETIS)

  cinch_add_devel_target(coloring
    SOURCES test/coloring.cc
    INPUTS
      test/simple2d-8x8.msh
      test/simple2d-16x16.msh
      test/simple2d-32x32.msh
      test/simple2d-48x48.msh
    LIBRARIES
      ${CINCH_RUNTIME_LIBRARIES}
      ${COLORING_LIBRARIES}
    POLICY ${UNIT_POLICY}
    THREADS 5
    FOLDER "Tests/Coloring/Devel"
  )

endif(ENABLE_PARMETIS)
