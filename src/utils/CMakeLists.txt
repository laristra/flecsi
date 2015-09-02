#-----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#-----------------------------------------------------------------------------~#

set(utils_HEADERS
  dummy.h
  PARENT_SCOPE
)

set(utils_SOURCES
  dummy.cc
  PARENT_SCOPE
)

#if(ENABLE_UNIT_TESTS)
#  cinch_add_unit(casename
#    SOURCES testfile.cc
#    LIBRARIES list
#    INCLUDES list
#    POLICY MPI
#    THREADS 1 2 4
#  )
#endif(ENABLE_UNIT_TESTS)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
