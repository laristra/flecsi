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

set(mesh_HEADERS
  mesh_topology.h
  mesh_types.h
  mesh_utils.h
  tree_topology.h
  PARENT_SCOPE)

set(mesh_SOURCES
  PARENT_SCOPE)

#------------------------------------------------------------------------------#
# Mesh unit tests.
#------------------------------------------------------------------------------#

cinch_add_unit(traversal
  SOURCES test/traversal.cc
  INPUTS test/traversal.blessed)

cinch_add_unit(dual
  SOURCES test/dual.cc
  INPUTS test/dual.blessed)

cinch_add_unit(bindings
  SOURCES test/bindings.cc
  INPUTS test/bindings.blessed)

#cinch_add_unit(serialize
#  SOURCES test/serialize.cc
#  INPUTS test/serialize.blessed)

#------------------------------------------------------------------------------#
# N-Tree unit tests.
#------------------------------------------------------------------------------#

cinch_add_unit(tree
  SOURCES test/tree.cc
  INPUTS test/tree.blessed)

cinch_add_unit(gravity
  SOURCES test/gravity.cc)

cinch_add_unit(gravity-state
  SOURCES test/gravity-state.cc)

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
