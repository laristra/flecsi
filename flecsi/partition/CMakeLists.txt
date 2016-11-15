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
  init_partitions_task.h
  partitioner.h
  PARENT_SCOPE
)

# Add MPI/Legion specific files

if(FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")
set(partition_HEADERS
  ${partition_HEADERS}
  init_partitions_task.h
  PARENT_SCOPE
)

set(partition_SOURCES
  ${partition_SOURCES}
  init_partitions_task.cc
  PARENT_SCOPE
)
endif()

cinch_add_unit(partition
  SOURCES test/partition.cc
  INPUTS  test/metis_mesh.blessed 
          test/metis_mesh_int64.blessed 
          test/metis.blessed 
          test/metis_int64.blessed 
          test/scotch_v5.blessed
          test/scotch.blessed
  LIBRARIES ${PARTITION_LIBRARIES} flecsi
)


if(FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")
#  cinch_add_unit(index_partition
#    SOURCES test/index_partition.cc
#    INPUTS test/index_partition.blessed
#    POLICY MPILEGION
#    THREADS 2
#    LIBRARIES flecsi)
endif()

endif()

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
