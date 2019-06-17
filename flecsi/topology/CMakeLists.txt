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
# Copyright (c) 2016, Triad National Security, LLC
# All rights reserved
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
# Add header files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(topology_HEADERS
  base_topology_types.h
  common/array_buffer.h
  common/connectivity.h
  common/entity_storage.h
  common/index_space.h
  common/utility_types.h
  internal/global.h
  internal/index.h
  ntree/interface.h
  ntree/types.h
  set/interface.h
  set/types.h
  structured_mesh/interface.h
  structured_mesh/types.h
  unstructured_mesh/interface.h
  unstructured_mesh/partition.h
  unstructured_mesh/storage.h
  unstructured_mesh/types.h
  unstructured_mesh/utils.h
)

set(topology_SOURCES
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(topology_HEADERS
    ${topology_HEADERS}
  )

  set(topology_SOURCES
    ${topology_SOURCES}
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(topology_HEADERS
    ${topology_HEADERS}
  )

  set(topology_SOURCES
    ${topology_SOURCES}
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(topology_HEADERS
    ${topology_HEADERS}
  )

  set(topology_SOURCES
    ${topology_SOURCES}
  )

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(topology_HEADERS
  ${topology_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(topology_SOURCES
  ${topology_SOURCES}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

#--------------------------------  NTREE  -------------------------------------#
ftest_add_unit(ntree
  SOURCES ntree/test/ntree.cc
)
ftest_add_unit(ntree_geometry
  SOURCES ntree/test/geometry.cc
)
