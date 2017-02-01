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
  PARENT_SCOPE
)

set(topology_SOURCES
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Mesh unit tests.
#------------------------------------------------------------------------------#

cinch_add_unit(traversal
  SOURCES
    test/traversal.cc
  INPUTS
    test/traversal.blessed
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
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
  INPUTS
    test/bindings.blessed
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
)

#------------------------------------------------------------------------------#
# N-Tree unit tests.
#------------------------------------------------------------------------------#

cinch_add_unit(tree
  SOURCES
    test/tree.cc
    test/pseudo_random.h
  INPUTS
    test/tree.blessed
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
)

cinch_add_unit(tree1d
  SOURCES
    test/tree1d.cc
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
)

cinch_add_unit(tree3d
  SOURCES
    test/tree3d.cc
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
)

cinch_add_unit(gravity
  SOURCES
    test/gravity.cc test/pseudo_random.h
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
)

cinch_add_unit(gravity-state
  SOURCES
    test/gravity-state.cc
    test/pseudo_random.h
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
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
