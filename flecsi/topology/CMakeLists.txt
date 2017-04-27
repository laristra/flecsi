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

set(topology_HEADERS
  graph_definition.h
  graph_utils.h
  index_space.h
  mesh_topology.h
  mesh_types.h
  mesh_utils.h
  tree_topology.h
)

cinch_install_headers(
  FILES ${topology_HEADERS}
  DESTINATION include/flecsi/topology)

#------------------------------------------------------------------------------#
# Mesh unit tests.
#------------------------------------------------------------------------------#

cinch_add_unit(traversal
  SOURCES
    test/traversal.cc
    $<TARGET_OBJECTS:data>
  INPUTS
    test/traversal.blessed
)

cinch_add_unit(dual
  SOURCES
    test/dual.cc
  INPUTS
    test/dual.blessed
)

cinch_add_unit(bindings
  SOURCES
    test/bindings.cc
    $<TARGET_OBJECTS:data>
  INPUTS
    test/bindings.blessed
)

#------------------------------------------------------------------------------#
# N-Tree unit tests.
#------------------------------------------------------------------------------#

cinch_add_unit(tree
  SOURCES
    test/tree.cc
    test/pseudo_random.h
    $<TARGET_OBJECTS:data>
  INPUTS
    test/tree.blessed
)

cinch_add_unit(tree1d
  SOURCES
    test/tree1d.cc
    $<TARGET_OBJECTS:data>
)

cinch_add_unit(tree3d
  SOURCES
    test/tree3d.cc
    $<TARGET_OBJECTS:data>
)

cinch_add_unit(gravity
  SOURCES
    test/gravity.cc test/pseudo_random.h
    $<TARGET_OBJECTS:data>
)

cinch_add_unit(gravity-state
  SOURCES
    test/gravity-state.cc
    test/pseudo_random.h
    $<TARGET_OBJECTS:data>
)

cinch_add_unit(structured
  SOURCES
    test/structured.cc
)

#------------------------------------------------------------------------------#
# Misc. unit tests.
#------------------------------------------------------------------------------#

cinch_add_unit(graph
  SOURCES
    test/graph.cc
)

cinch_add_unit(devel-graph
  SOURCES
    test/devel-graph.cc
  POLICY
    SERIAL_DEVEL
)

cinch_add_unit(index-space
  SOURCES
    test/index-space.cc test/pseudo_random.h
)

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
