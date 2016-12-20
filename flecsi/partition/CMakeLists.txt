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

if(ENABLE_PARTITION)

set(partition_HEADERS
  index_partition.h
  partitioner.h
  PARENT_SCOPE
)

#FIXME: remove the check for Parmetis when there are other kinds of partitioner
#implemented

if(PARMETIS_FOUND)
cinch_add_unit(dcrs
  SOURCES test/dcrs.cc
  INPUTS
    test/simple2d-8x8.msh
    test/simple2d-16x16.msh
  LIBRARIES ${PARTITION_LIBRARIES}
  POLICY MPI
  THREADS 5
)

cinch_add_unit(partition
  SOURCES test/partition.cc
  INPUTS
    test/simple2d-8x8.msh
    test/simple2d-16x16.msh
    test/simple2d-32x32.msh
    test/simple2d-48x48.msh
  LIBRARIES ${PARTITION_LIBRARIES}
  POLICY MPI
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
endif(PARMETIS_FOUND)


#cinch_add_unit(partition
#  SOURCES test/partition.cc
#  INPUTS  test/metis_mesh.blessed 
#          test/metis_mesh_int64.blessed 
#          test/metis.blessed 
#          test/metis_int64.blessed 
#          test/scotch_v5.blessed
#          test/scotch.blessed
#  LIBRARIES ${PARTITION_LIBRARIES} flecsi
#)

#if(FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")
#  cinch_add_unit(index_partition
#    SOURCES test/index_partition.cc
#    INPUTS test/index_partition.blessed
#    POLICY MPILEGION
#    THREADS 2
#    LIBRARIES flecsi)
#endif()

endif()

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
