#-----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#-----------------------------------------------------------------------------~#

set(utils_HEADERS
  dummy.h
  PARENT_SCOPE)

set(utils_SOURCES
  dummy.cc
  PARENT_SCOPE)

cinch_add_unit(bitfield
  SOURCES test/bitfield.cc)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
