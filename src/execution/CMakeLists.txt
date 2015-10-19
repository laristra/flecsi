#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

set(execution_HEADERS
  context.h
  task.h
  PARENT_SCOPE
)

#set(execution_SOURCES
#  PARENT_SCOPE
#)

cinch_add_unit(task
  SOURCES test/task.cc
)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
