#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

if(ENABLE_PARTITION)

set(partition_HEADERS
  partitioner.h
  PARENT_SCOPE
)

#set(partition_SOURCES
#  PARENT_SCOPE
#)

cinch_add_unit(partition
  SOURCES test/partition.cc
  INPUTS  test/metis_mesh.blessed 
          test/metis.blessed 
          test/scotch.blessed
  LIBRARIES ${PARTITION_LIBRARIES}
)
endif()

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
