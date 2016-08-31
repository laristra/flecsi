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
  SOURCES test/tree.cc
  INPUTS test/tree.blessed)

cinch_add_unit(tree1d
  SOURCES test/tree1d.cc)

cinch_add_unit(gravity
  SOURCES test/gravity.cc)

cinch_add_unit(gravity-state
  SOURCES test/gravity-state.cc)

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
