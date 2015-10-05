#-----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#-----------------------------------------------------------------------------~#

set(specializations_HEADERS
  burton_types.h
  burton.h
  PARENT_SCOPE
)

#set(specializations_SOURCES
#  PARENT_SCOPE
#)

cinch_add_unit(burton
  SOURCES test/burton.cc
  INPUT test/burton.blessed)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
