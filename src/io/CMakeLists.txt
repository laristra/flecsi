#-----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#-----------------------------------------------------------------------------~#

if(ENABLE_IO)

set(io_HEADERS
  io.h
  burton_io.h
  PARENT_SCOPE
)
  
cinch_add_unit(burton_io
  SOURCES test/burton_io.cc
  LIBRARIES ${IO_LIBRARIES})

endif(ENABLE_IO)
