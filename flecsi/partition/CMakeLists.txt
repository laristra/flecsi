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

set(partition_HEADERS
  communicator.h
  dcrs.h
  index_partition.h
  partitioner.h
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
  set(partition_HEADERS
    ${partition_HEADERS}
    dcrs_utils.h
    mpi_communicator.h
    mpi_utils.h
  )
endif()

if(ENABLE_PARMETIS)
  set(partition_HEADERS
    ${partition_HEADERS}
    parmetis_partitioner.h
    weaver.h
  )
endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(partition_HEADERS
    ${partition_HEADERS}
    PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(partition_SOURCES
    ${partition_SOURCES}
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
  LIBRARIES ${PARTITION_LIBRARIES}
  POLICY MPI
  THREADS 5
)

cinch_add_unit(devel-dcrs
  SOURCES test/devel-dcrs.cc
  INPUTS
    test/simple2d-8x8.msh
    test/simple2d-16x16.msh
  LIBRARIES ${PARTITION_LIBRARIES}
  POLICY MPI_DEVEL
  THREADS 5
)

# Both of these tests depend on ParMETIS.
# This could change if we add more partitioner types.
if(ENABLE_PARTITIONING AND ENABLE_PARMETIS)

cinch_add_unit(partition
  SOURCES test/partition.cc
  INPUTS
    test/simple2d-8x8.msh
    test/simple2d-16x16.msh
    test/simple2d-32x32.msh
    test/simple2d-48x48.msh
  LIBRARIES ${PARTITION_LIBRARIES}
  POLICY MPI_DEVEL
  THREADS 5
)

cinch_add_unit(weaver
  SOURCES test/weaver.cc
  INPUTS
    test/simple2d-8x8.msh
    test/simple2d-16x16.msh
    test/simple2d-32x32.msh
    test/simple2d-48x48.msh
  LIBRARIES ${PARTITION_LIBRARIES}
  POLICY MPI
  THREADS 5
)

endif()

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
