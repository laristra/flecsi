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

set(partition_HEADERS
  communicator.h
  dcrs.h
  index_partition.h
  partitioner.h
)

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

set(partition_HEADERS
  ${partition_HEADERS}
  PARENT_SCOPE)

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
