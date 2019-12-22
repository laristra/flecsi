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
  base.hh
  array_buffer.hh
  canonical/interface.hh
  canonical/types.hh
  connectivity.hh
  entity_storage.hh
  index_space.hh
  utility_types.hh
  internal/global.hh
  internal/index.hh
  ntree/interface.hh
  ntree/types.hh
  set/interface.hh
  set/types.hh
  structured/interface.hh
  structured/types.hh
  unstructured/coloring.hh
  unstructured/definition.hh
  unstructured/interface.hh
  unstructured/partition.hh
  unstructured/simple_definition.hh
  unstructured/storage.hh
  unstructured/types.hh
  unstructured/utils.hh
)

set(topology_SOURCES
  canonical/types.cc
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

#---------------------------   Unstructured Mesh  -----------------------------#

ftest_add_unit(umesh_coloring
  SOURCES unstructured/test/coloring.cc
)

ftest_add_unit(simple_definition
  SOURCES unstructured/test/simple_definition.cc
  INPUTS unstructured/test/simple2d-16x16.msh
)

#----------------------------------  NTREE  -----------------------------------#

ftest_add_unit(ntree
  SOURCES ntree/test/ntree.cc
  INPUTS ntree/test/coordinates.blessed
)

ftest_add_unit(ntree_geometry
  SOURCES ntree/test/geometry.cc
)
