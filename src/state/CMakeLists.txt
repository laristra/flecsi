#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

set(state_HEADERS
  state.h
  PARENT_SCOPE
)

#set(state_SOURCES
#  PARENT_SCOPE
#)

cinch_add_unit(state
  SOURCES test/state.cc)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
