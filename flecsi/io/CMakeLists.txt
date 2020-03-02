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
  backend.hh
)

set(io_SOURCES
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(io_HEADERS
    leg/policy.hh
    ${io_HEADERS}
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

elseif(FLECSI_RUNTIME_MODEL STREQUAL "charm")

  set(io_HEADERS
    charm/policy.hh
    ${io_HEADERS}
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

add_unit(io
  SOURCES
    test/leg/io.cc
  THREADS 4
)
elseif(FLECSI_RUNTIME_MODEL STREQUAL "charm")

ftest_add_unit(io
  SOURCES
    test/charm/io.cc
  THREADS 4
)
endif()
        
add_unit(io_index
  SOURCES
    test/io_index.cc
  THREADS 4
)

add_unit(io_metadata
  SOURCES
    test/io_metadata.cc
  THREADS 2
)
                
endif()
