#------------------------------------------------------------------------------#
#  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
# /@@/////  /@@          @@////@@ @@////// /@@
# /@@       /@@  @@@@@  @@    // /@@       /@@
# /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
# /@@////   /@@/@@@@@@@/@@       ////////@@/@@
# /@@       /@@/@@//// //@@    @@       /@@/@@
# /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
# //       ///  //////   //////  ////////  //
#
# Copyright (c) 2016, Triad National Security, LLC
# All rights reserved
#------------------------------------------------------------------------------#

if(ENABLE_HDF5)

set(io_HEADERS
  io_interface.hh
)

set(io_SOURCES
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(io_HEADERS
    legion/io_policy.hh
    ${io_HEADERS}
  )

  set(io_SOURCES
    legion/io_policy.cc
    ${io_SOURCES}
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(io_HEADERS
    ${io_HEADERS}
  )

  set(io_SOURCES
    ${io_SOURCES}
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(io_HEADERS ${io_HEADERS}
  )

  set(io_SOURCES
    ${io_SOURCES}
  )

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(io_HEADERS
  ${io_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(io_SOURCES
  ${io_SOURCES}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")
ftest_add_unit(io
  SOURCES
    test/legion/io.cc
)
endif()
        
ftest_add_unit(io_index
  SOURCES
    test/io_index.cc
)

ftest_add_unit(io_metadata
  SOURCES
    test/io_metadata.cc
)
                
endif()
