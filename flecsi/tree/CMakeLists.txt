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

set(tree_HEADERS
  tree_topology.h
  PARENT_SCOPE)

set(tree_SOURCES
  PARENT_SCOPE)

cinch_add_unit(tree
  SOURCES test/tree.cc test/pseudo_random.h
  INPUTS test/tree.blessed)

cinch_add_unit(tree1d
  SOURCES test/tree1d.cc)

cinch_add_unit(tree3d
  SOURCES test/tree3d.cc)

cinch_add_unit(gravity
  SOURCES test/gravity.cc test/pseudo_random.h)

cinch_add_unit(gravity-state
  SOURCES test/gravity-state.cc test/pseudo_random.h)

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
