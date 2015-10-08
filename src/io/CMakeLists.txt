#-----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#-----------------------------------------------------------------------------~#

if(ENABLE_IO)

set(io_HEADERS
  io.h
  io_base.h
  io_exodus.h
  PARENT_SCOPE
)
  
cinch_add_unit(io
  SOURCES test/io.cc
  LIBRARIES ${IO_LIBRARIES})

endif(ENABLE_IO)
