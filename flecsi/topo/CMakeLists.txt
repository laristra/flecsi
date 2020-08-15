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
  ntree/hash_table.hh
  ntree/interface.hh
  ntree/storage.hh
  ntree/types.hh
  set/interface.hh
  set/types.hh
  structured/box_types.hh
  structured/box_utils.hh
  structured/dependent_entities_colorer.hh
  structured/interface.hh
  structured/simple_box_colorer.hh
  structured/types.hh
  unstructured/coloring_utils.hh
  unstructured/definition.hh
  unstructured/interface.hh
  unstructured/parallel_definition.hh
  unstructured/simple_definition.hh
  unstructured/types.hh
  unstructured/closure_utils.hh
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

#---------------------------   Structured Mesh  -----------------------------#

add_unit(test_interface
  SOURCES structured/test/test_interface.cc
  INPUTS
     structured/test/smesh_1d_0.blessed
     structured/test/smesh_1d_1.blessed
  THREADS 2 
)

#---------------------------   Unstructured Mesh  -----------------------------#

add_unit(coloring
  SOURCES unstructured/test/coloring.cc
  INPUTS
    unstructured/test/simple2d-16x16.msh
    unstructured/test/simple2d-8x8.msh
  THREADS 5
)

#----------------------------------  NTREE  -----------------------------------#

add_unit(ntree
  SOURCES ntree/test/ntree.cc
  INPUTS ntree/test/coordinates.blessed
)

add_unit(ntree_geometry
  SOURCES ntree/test/geometry.cc
)
