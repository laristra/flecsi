#-----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#-----------------------------------------------------------------------------~#

set(base_HEADERS
  mesh_entity.h
  vertex.h
  PARENT_SCOPE
)

#set(base_SOURCES
#  PARENT_SCOPE
#)

if(ENABLE_UNIT_TESTS)
  cinch_add_unit(vertex
    SOURCES vertex_test.cc
  )
endif(ENABLE_UNIT_TESTS)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
