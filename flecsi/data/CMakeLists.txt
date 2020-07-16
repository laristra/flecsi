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

#------------------------------------------------------------------------------#
# Add header files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(data_HEADERS
  accessor.hh
  backend.hh
  coloring.hh
  reference.hh
  field.hh
  field_info.hh
  layout.hh
  privilege.hh
  topology_accessor.hh
  topology_slot.hh
  topology.hh
)

set(data_SOURCES
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(data_HEADERS
    leg/policy.hh
    ${data_HEADERS}
  )

  set(data_SOURCES
    ${data_SOURCES}
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(data_HEADERS
    mpi/policy.hh
    ${data_HEADERS}
  )

  set(data_SOURCES
    ${data_SOURCES}
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(data_HEADERS ${data_HEADERS}
  )

  set(data_SOURCES
    ${data_SOURCES}
  )

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(data_HEADERS
  ${data_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(data_SOURCES
  ${data_SOURCES}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

if(NOT FLECSI_RUNTIME_MODEL STREQUAL "mpi")
add_unit(canonical
  SOURCES
    test/canonical.cc
  THREADS 2
)

add_unit(index
  SOURCES
    test/index.cc
  THREADS 2
)
endif()

#add_unit(structured
#  SOURCES
#    test/structured.cc
#  THREADS 2
#)
