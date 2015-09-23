#-----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#-----------------------------------------------------------------------------~#

set(base_HEADERS
  mesh_entity.h
  mesh_topology.h
  vertex.h
  PARENT_SCOPE)

set(base_SOURCES
  PARENT_SCOPE)

cinch_add_unit(traversal
  SOURCES unit/traversal.cc
  INPUT unit/traversal.blessed)

cinch_add_unit(dual
  SOURCES unit/dual.cc
  INPUT unit/dual.blessed)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
