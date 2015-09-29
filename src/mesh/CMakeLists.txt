#-----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#-----------------------------------------------------------------------------~#

set(mesh_HEADERS
  mesh_entity.h
  mesh_topology.h
  vertex.h
  PARENT_SCOPE)

set(mesh_SOURCES
  PARENT_SCOPE)

cinch_add_unit(traversal
  SOURCES test/traversal.cc
  INPUT test/traversal.blessed)

cinch_add_unit(dual
  SOURCES test/dual.cc
  INPUT test/dual.blessed)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
