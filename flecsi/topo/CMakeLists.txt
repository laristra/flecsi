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

set(topo_HEADERS
  array_buffer.hh
  canonical/interface.hh
  canonical/types.hh
  connectivity.hh
  core.hh
  entity_storage.hh
  global.hh
  index.hh
  index_space.hh
  ntree/coloring.hh
  ntree/geometry.hh
  ntree/interface.hh
  ntree/storage.hh
  ntree/types.hh
  set/interface.hh
  set/types.hh
  structured/interface.hh
  structured/types.hh
  unstructured/closure_utils.hh
  unstructured/coloring.hh
  unstructured/coloring_types.hh
  unstructured/dcrs_utils.hh
  unstructured/definition.hh
  unstructured/index_coloring.hh
  unstructured/interface.hh
  unstructured/parallel_definition.hh
  unstructured/partition.hh
  unstructured/simple_definition.hh
  unstructured/storage.hh
  unstructured/types.hh
  unstructured/utils.hh
  utility_types.hh
)

set(topo_SOURCES
)

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(topo_HEADERS
  ${topo_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(topo_SOURCES
  ${topo_SOURCES}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

#---------------------------   Unstructured Mesh  -----------------------------#

add_unit(dcrs
  SOURCES unstructured/test/dcrs.cc
  INPUTS
    unstructured/test/simple2d-16x16.msh
    unstructured/test/simple2d-8x8.msh
  THREADS 5
)

#----------------------------------  NTREE  -----------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

add_unit(ntree_policy
  SOURCES ntree/test/ntree.cc
  INPUTS ntree/test/coordinates.blessed
  THREADS 4
)

endif()


add_unit(ntree_geometry
  SOURCES ntree/test/geometry.cc
)
