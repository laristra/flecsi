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
  base_topology_types.hh
  common/array_buffer.hh
  common/connectivity.hh
  common/entity_storage.hh
  common/index_space.hh
  common/utility_types.hh
  internal/global.hh
  internal/index.hh
  ntree/interface.hh
  ntree/types.hh
  set/interface.hh
  set/types.hh
  structured_mesh/interface.hh
  structured_mesh/types.hh
  unstructured_mesh/interface.hh
  unstructured_mesh/partition.hh
  unstructured_mesh/storage.hh
  unstructured_mesh/types.hh
  unstructured_mesh/utils.hh
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
