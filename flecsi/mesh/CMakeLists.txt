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
  PARENT_SCOPE)

set(mesh_SOURCES
  PARENT_SCOPE)

cinch_add_unit(traversal
  SOURCES test/traversal.cc
  INPUTS test/traversal.blessed)

cinch_add_unit(dual
  SOURCES test/dual.cc
  INPUTS test/dual.blessed)

cinch_add_unit(bindings
  SOURCES test/bindings.cc
  INPUTS test/bindings.blessed)

#cinch_add_unit(bindings2
#  SOURCES test/bindings2.cc
#  INPUTS test/bindings.blessed)

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
