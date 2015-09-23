#-----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#-----------------------------------------------------------------------------~#

set(geometry_HEADERS
  point.h
  PARENT_SCOPE
)

#set(geometry_SOURCES
#  PARENT_SCOPE
#)

cinch_add_unit(point
  SOURCES unit/point.cc)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
