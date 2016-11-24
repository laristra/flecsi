#~----------------------------------------------------------------------------~#
#  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
# /@@/////  /@@          @@////@@ @@////// /@@
# /@@       /@@  @@@@@  @@    // /@@       /@@
# /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
# /@@////   /@@/@@@@@@@/@@       ////////@@/@@
# /@@       /@@/@@//// //@@    @@       /@@/@@
# /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
# //       ///  //////   //////  ////////  // 
# 
# Copyright (c) 2016 Los Alamos National Laboratory, LLC
# All rights reserved
#~----------------------------------------------------------------------------~#

if(ENABLE_IO)

set(io_HEADERS
  io.h
  io_base.h
  io_exodus.h
  PARENT_SCOPE
)
  
set(io_SOURCES
  PARENT_SCOPE
)

cinch_add_unit(io
  SOURCES test/io.cc
)

cinch_add_unit(definition
  SOURCES test/definition.cc
  INPUTS test/simple2d-8x8.msh test/simple2d-4x4.msh
)

cinch_add_unit(dCRS
  SOURCES test/dCRS.cc
  INPUTS test/simple2d-8x8.msh test/simple2d-4x4.msh
  LIBRARIES ${PARTITION_LIBRARIES}
  POLICY MPI
  THREADS 5
)

endif(ENABLE_IO)

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
