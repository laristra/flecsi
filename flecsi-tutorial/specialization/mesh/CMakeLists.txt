#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

set(mesh_HEADERS
  coloring.h
  entity_types.h
  mesh.h
  tasks.h
  types.h
  PARENT_SCOPE
)

set(mesh_SOURCES
  coloring.cc
  control_points.cc
  mesh.cc
  tasks.cc
  PARENT_SCOPE
)
