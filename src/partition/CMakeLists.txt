#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

set(partition_HEADERS
  partitioner.h
  PARENT_SCOPE
)

#set(partition_SOURCES
#  PARENT_SCOPE
#)

cinch_add_unit(partition
  SOURCES test/partition.cc
  LIBRARIES ${PARTITION_LIBRARIES})

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
